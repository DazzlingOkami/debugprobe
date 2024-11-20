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

#include "general.h"
#include "platform.h"
#include "morse.h"
#include "exception.h"

static void platform_gpio_init(void *port, int pin, int is_out, int value){
    (void) port;
    gpio_init(pin);
    gpio_set_dir(pin, is_out ? GPIO_OUT : GPIO_IN);
    gpio_put(pin, value);
    gpio_set_slew_rate(pin, GPIO_SLEW_RATE_FAST);
}

void platform_init(void)
{
    platform_timing_init();
    platform_gpio_init(PWR_BR_PORT, PWR_BR_PIN, 1, 0);

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    probe_init();
}

void platform_nrst_set_val(bool assert)
{
}

bool platform_nrst_get_val(void)
{
    return false;
}

const char *platform_target_voltage(void)
{
    static char ret[] = "0.0V";
    uint32_t val = platform_target_voltage_sense();
    ret[0] = '0' + val / 10U;
    ret[2] = '0' + val % 10U;

    return ret;
}

#ifdef PLATFORM_HAS_POWER_SWITCH
bool platform_target_get_power(void)
{
    return gpio_read(PWR_BR_PORT, PWR_BR_PIN);
}

bool platform_target_set_power(const bool power)
{
    gpio_set_val(PWR_BR_PORT, PWR_BR_PIN, power);
    return true;
}

uint32_t platform_target_voltage_sense(void)
{
    uint32_t result = adc_read();
    return (result * 97u) / 8192u;
}
#endif

void platform_target_clk_output_enable(bool enable)
{
    (void)enable;
}

bool platform_spi_init(const spi_bus_e bus)
{
    (void)bus;
    return false;
}

bool platform_spi_deinit(const spi_bus_e bus)
{
    (void)bus;
    return false;
}

bool platform_spi_chip_select(const uint8_t device_select)
{
    (void)device_select;
    return false;
}

uint8_t platform_spi_xfer(const spi_bus_e bus, const uint8_t value)
{
    (void)bus;
    return value;
}

int platform_hwversion(void)
{
    return 0;
}
