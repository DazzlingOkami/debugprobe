/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 DazzlingOkami
 * Written by DazzlingOkami <kinghd1912@163.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <pico/stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "tusb.h"
#include "probe_config.h"
#include "ringbuf.h"
#include "hardware/dma.h"

#define RX_RINGBUF_SIZE (256)
#define TX_RINGBUF_SIZE (1024)

RINGBUF_STATIC_ALLOC(rx_ringbuf, RX_RINGBUF_SIZE);
RINGBUF_STATIC_ALLOC(tx_ringbuf, TX_RINGBUF_SIZE);

TaskHandle_t uart_taskhandle;

TickType_t last_wake, interval = 100;

// Actually s^-1 so 25ms
#define DEBOUNCE_MS 40
static uint debounce_ticks = 5;

#ifdef PROBE_UART_TX_LED
static volatile uint tx_led_debounce;
#endif

#ifdef PROBE_UART_RX_LED
static uint rx_led_debounce;
#endif

#define CDC_INTERFACE 0

#define DMA_OR_IRQ 1    // 1:DMA 0:IRQ

#if DMA_OR_IRQ
static int uart_tx_dma_ch;
static int uart_rx_dma_ch;

static int tx_dma_total = 0;
static int rx_dma_total = 0;

static void dma_irq0_handler() {
    if(dma_channel_get_irq0_status(uart_tx_dma_ch)) {
        dma_channel_acknowledge_irq0(uart_tx_dma_ch);

        ringbuf_consume(&tx_ringbuf, tx_dma_total);

        int tx_len = 0;
        const char *tx_buf = ringbuf_get_ptr(&tx_ringbuf, &tx_len);
        if(tx_len > 0){
            tx_dma_total = tx_len;
            dma_channel_transfer_from_buffer_now(uart_tx_dma_ch, tx_buf, tx_len);
        }
    }

    if(dma_channel_get_irq0_status(uart_rx_dma_ch)) {
        dma_channel_acknowledge_irq0(uart_rx_dma_ch);

        int dma_xfer_len = rx_dma_total - dma_channel_hw_addr(uart_rx_dma_ch)->transfer_count;
        ringbuf_produce(&rx_ringbuf, dma_xfer_len);

        char* rx_buf = ringbuf_puts_ptr(&rx_ringbuf, &rx_dma_total);
        dma_channel_transfer_to_buffer_now(uart_rx_dma_ch, rx_buf, rx_dma_total);
    }
}
#else
static void cdc_uart_irq_handler(void){
    if(uart_get_hw(PROBE_UART_INTERFACE)->ris & UART_UARTRIS_TXRIS_BITS){
        uart_get_hw(PROBE_UART_INTERFACE)->icr |= UART_UARTICR_TXIC_BITS;
        while(uart_is_writable(PROBE_UART_INTERFACE)){
            int c = ringbuf_get(&tx_ringbuf);
            if(c < 0)
                break;
            uart_get_hw(PROBE_UART_INTERFACE)->dr = (char)c;
        }
    }
    if(uart_get_hw(PROBE_UART_INTERFACE)->ris & (UART_UARTRIS_RXRIS_BITS | UART_UARTRIS_RTRIS_BITS)){
        while(uart_is_readable(PROBE_UART_INTERFACE)){
            ringbuf_put(&rx_ringbuf, (char)uart_get_hw(PROBE_UART_INTERFACE)->dr);
        }
        uart_get_hw(PROBE_UART_INTERFACE)->icr |= (UART_UARTICR_RTIC_BITS | UART_UARTICR_RXIC_BITS);
    }
    if(uart_get_hw(PROBE_UART_INTERFACE)->ris){
        uart_get_hw(PROBE_UART_INTERFACE)->icr |= 0xFFFFFFFF;
    }
}
#endif

void cdc_uart_init(void) {
    gpio_set_function(PROBE_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PROBE_UART_RX, GPIO_FUNC_UART);
    gpio_set_pulls(PROBE_UART_TX, 1, 0);
    gpio_set_pulls(PROBE_UART_RX, 1, 0);
    uart_init(PROBE_UART_INTERFACE, PROBE_UART_BAUDRATE);

#ifdef PROBE_UART_HWFC
    /* HWFC implies that hardware flow control is implemented and the
     * UART operates in "full-duplex" mode (See USB CDC PSTN120 6.3.12).
     * Default to pulling in the active direction, so an unconnected CTS
     * behaves the same as if CTS were not enabled. */
    gpio_set_pulls(PROBE_UART_CTS, 0, 1);
    gpio_set_function(PROBE_UART_RTS, GPIO_FUNC_UART);
    gpio_set_function(PROBE_UART_CTS, GPIO_FUNC_UART);
    uart_set_hw_flow(PROBE_UART_INTERFACE, true, true);
#else
#ifdef PROBE_UART_RTS
    gpio_init(PROBE_UART_RTS);
    gpio_set_dir(PROBE_UART_RTS, GPIO_OUT);
    gpio_put(PROBE_UART_RTS, 1);
#endif
#endif

#ifdef PROBE_UART_DTR
    gpio_init(PROBE_UART_DTR);
    gpio_set_dir(PROBE_UART_DTR, GPIO_OUT);
    gpio_put(PROBE_UART_DTR, 1);
#endif

#if DMA_OR_IRQ == 0
    uart_set_irq_enables(PROBE_UART_INTERFACE, true, true);
    irq_set_exclusive_handler(UART1_IRQ, cdc_uart_irq_handler);
    irq_set_enabled(UART1_IRQ, true);
#else
    uart_tx_dma_ch = dma_claim_unused_channel(true);
    uart_rx_dma_ch = dma_claim_unused_channel(true);

    dma_channel_config tx_config = dma_channel_get_default_config(uart_tx_dma_ch);
    channel_config_set_transfer_data_size(&tx_config, DMA_SIZE_8);
    channel_config_set_read_increment(&tx_config, true);
    channel_config_set_write_increment(&tx_config, false);
    channel_config_set_dreq(&tx_config, uart_get_dreq_num(PROBE_UART_INTERFACE, true));
    dma_channel_set_write_addr(uart_tx_dma_ch, &(uart_get_hw(PROBE_UART_INTERFACE)->dr), false);
    dma_channel_set_config(uart_tx_dma_ch, &tx_config, false);
    dma_channel_set_irq0_enabled(uart_tx_dma_ch, true);

    dma_channel_config rx_config = dma_channel_get_default_config(uart_rx_dma_ch);
    channel_config_set_transfer_data_size(&rx_config, DMA_SIZE_8);
    channel_config_set_read_increment(&rx_config, false);
    channel_config_set_write_increment(&rx_config, true);
    channel_config_set_dreq(&rx_config, uart_get_dreq_num(PROBE_UART_INTERFACE, false));
    dma_channel_set_read_addr(uart_rx_dma_ch, &(uart_get_hw(PROBE_UART_INTERFACE)->dr), false);
    dma_channel_set_config(uart_rx_dma_ch, &rx_config, false);
    dma_channel_set_irq0_enabled(uart_rx_dma_ch, true);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq0_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    /* start dma recv */
    char* rx_buf = ringbuf_puts_ptr(&rx_ringbuf, &rx_dma_total);
    dma_channel_transfer_to_buffer_now(uart_rx_dma_ch, rx_buf, rx_dma_total);
#endif
}

bool cdc_task(void){
    bool keep_alive = false;

#ifdef PROBE_UART_TX_LED
    if (tx_led_debounce)
        tx_led_debounce--;
    else
        gpio_put(PROBE_UART_TX_LED, 0);
#endif
#ifdef PROBE_UART_RX_LED
    if (rx_led_debounce)
        rx_led_debounce--;
    else
        gpio_put(PROBE_UART_RX_LED, 0);
#endif

    if(tud_cdc_n_available(CDC_INTERFACE)){
        char* tx_buf;
        int tx_buf_len;
        tx_buf = ringbuf_puts_ptr(&tx_ringbuf, &tx_buf_len);
        int xfer_len = MIN(tx_buf_len, tud_cdc_n_available(CDC_INTERFACE));
        if(xfer_len > 0){
            tud_cdc_n_read(CDC_INTERFACE, tx_buf, xfer_len);
            ringbuf_produce(&tx_ringbuf, xfer_len);

#if DMA_OR_IRQ == 0
            while(uart_is_writable(PROBE_UART_INTERFACE) && ringbuf_elements(&tx_ringbuf) > 0){
                char c = ringbuf_get(&tx_ringbuf);
                uart_get_hw(PROBE_UART_INTERFACE)->dr = c;
            }
#else
            irq_set_enabled(DMA_IRQ_0, false);
            if(dma_channel_is_busy(uart_tx_dma_ch) == false &&
               dma_channel_get_irq0_status(uart_tx_dma_ch) == false){
                int tx_len = 0;
                const char *dma_buf = ringbuf_get_ptr(&tx_ringbuf, &tx_len);
                tx_dma_total = tx_len;
                if(tx_len > 0){
                    dma_channel_transfer_from_buffer_now(uart_tx_dma_ch, dma_buf, tx_len);
                }
            }
            irq_set_enabled(DMA_IRQ_0, true);
#endif
        }
        keep_alive = true;

#ifdef PROBE_UART_TX_LED
        gpio_put(PROBE_UART_TX_LED, 1);
        tx_led_debounce = debounce_ticks;
#endif
    }

    if(tud_cdc_n_write_available(CDC_INTERFACE)){
#if DMA_OR_IRQ
        irq_set_enabled(DMA_IRQ_0, false);
        int dma_xfer_len = rx_dma_total - dma_channel_hw_addr(uart_rx_dma_ch)->transfer_count;
        if(dma_xfer_len > 0){
            ringbuf_produce(&rx_ringbuf, dma_xfer_len);
            rx_dma_total -= dma_xfer_len;
        }
        irq_set_enabled(DMA_IRQ_0, true);
#endif
        const char* rx_buf;
        int rx_buf_len;
        rx_buf = ringbuf_get_ptr(&rx_ringbuf, &rx_buf_len);
        int xfer_len = MIN(tud_cdc_n_write_available(CDC_INTERFACE), rx_buf_len);
        if(xfer_len > 0){
            tud_cdc_n_write(CDC_INTERFACE, rx_buf, xfer_len);
            tud_cdc_n_write_flush(CDC_INTERFACE);
            ringbuf_consume(&rx_ringbuf, xfer_len);
            keep_alive = true;

#ifdef PROBE_UART_RX_LED
            gpio_put(PROBE_UART_RX_LED, 1);
            rx_led_debounce = debounce_ticks;
#endif
        }
    }

    return keep_alive;
}

void cdc_thread(void *ptr)
{
    BaseType_t delayed;
    last_wake = xTaskGetTickCount();
    bool keep_alive;
    /* Threaded with a polling interval that scales according to linerate */
    while (1) {
        keep_alive = cdc_task();
        if (!keep_alive) {
        delayed = xTaskDelayUntil(&last_wake, interval);
            if (delayed == pdFALSE)
            last_wake = xTaskGetTickCount();
        }
    }
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* line_coding)
{
    if(itf != CDC_INTERFACE)
        return;
    uart_parity_t parity;
    uint data_bits, stop_bits;
    /* Set the tick thread interval to the amount of time it takes to
     * fill up half a FIFO. Millis is too coarse for integer divide.
     */
    uint32_t micros = (1000 * 1000 * 16 * 10) / MAX(line_coding->bit_rate, 1);
    /* Modifying state, so park the thread before changing it. */
    vTaskSuspend(uart_taskhandle);
    interval = MAX(1, micros / ((1000 * 1000) / configTICK_RATE_HZ));
    debounce_ticks = MAX(1, configTICK_RATE_HZ / (interval * DEBOUNCE_MS));
    probe_info("New baud rate %ld micros %ld interval %lu\n",
                                    line_coding->bit_rate, micros, interval);
    uart_deinit(PROBE_UART_INTERFACE);
    tud_cdc_n_write_clear(CDC_INTERFACE);
    tud_cdc_n_read_flush(CDC_INTERFACE);
    uart_init(PROBE_UART_INTERFACE, line_coding->bit_rate);
#if DMA_OR_IRQ == 0
    uart_set_irq_enables(PROBE_UART_INTERFACE, true, true);
#endif

    ringbuf_reset(&rx_ringbuf);
    ringbuf_reset(&tx_ringbuf);

    switch (line_coding->parity) {
    case CDC_LINE_CODING_PARITY_ODD:
        parity = UART_PARITY_ODD;
        break;
    case CDC_LINE_CODING_PARITY_EVEN:
        parity = UART_PARITY_EVEN;
        break;
    default:
        probe_info("invalid parity setting %u\n", line_coding->parity);
        /* fallthrough */
    case CDC_LINE_CODING_PARITY_NONE:
        parity = UART_PARITY_NONE;
        break;
    }

    switch (line_coding->data_bits) {
    case 5:
    case 6:
    case 7:
    case 8:
        data_bits = line_coding->data_bits;
        break;
    default:
        probe_info("invalid data bits setting: %u\n", line_coding->data_bits);
        data_bits = 8;
        break;
    }

    /* The PL011 only supports 1 or 2 stop bits. 1.5 stop bits is translated to 2,
     * which is safer than the alternative. */
    switch (line_coding->stop_bits) {
    case CDC_LINE_CONDING_STOP_BITS_1_5:
    case CDC_LINE_CONDING_STOP_BITS_2:
        stop_bits = 2;
    break;
    default:
        probe_info("invalid stop bits setting: %u\n", line_coding->stop_bits);
        /* fallthrough */
    case CDC_LINE_CONDING_STOP_BITS_1:
        stop_bits = 1;
    break;
    }

    uart_set_format(PROBE_UART_INTERFACE, data_bits, stop_bits, parity);
    vTaskResume(uart_taskhandle);
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    if(itf != CDC_INTERFACE)
        return;
#ifdef PROBE_UART_RTS
    gpio_put(PROBE_UART_RTS, !rts);
#endif
#ifdef PROBE_UART_DTR
    gpio_put(PROBE_UART_DTR, !dtr);
#endif

    /* CDC drivers use linestate as a bodge to activate/deactivate the interface.
     * Resume our UART polling on activate, stop on deactivate */
    if (!dtr && !rts) {
        vTaskSuspend(uart_taskhandle);
#ifdef PROBE_UART_RX_LED
        gpio_put(PROBE_UART_RX_LED, 0);
        rx_led_debounce = 0;
#endif
#ifdef PROBE_UART_TX_LED
        gpio_put(PROBE_UART_TX_LED, 0);
        tx_led_debounce = 0;
#endif
    } else
        vTaskResume(uart_taskhandle);
}
