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
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define rcc_ahb_frequency (125 * 1000 * 1000)

uint32_t target_clk_divider = 0;

void platform_timing_init(void)
{
}

void platform_delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

uint32_t platform_time_ms(void)
{
    return xTaskGetTickCount();
}

/*
 * Assume some USED_SWD_CYCLES per clock and CYCLES_PER_CNT cycles
 * per delay loop count with 2 delay loops per clock
 */

/* Values for rp2040 at 125 MHz */
#define USED_SWD_CYCLES 56
#define CYCLES_PER_CNT  8

void platform_max_frequency_set(const uint32_t frequency)
{
    if(frequency < 1000){
        probe_set_swclk_freq(1);
    }else{
        probe_set_swclk_freq(frequency / 1000);
    }
    
    /* For JTAG Bit-banging */
    uint32_t divisor = rcc_ahb_frequency - USED_SWD_CYCLES * frequency;
    /* If we now have an insanely big divisor, the above operation wrapped to a negative signed number. */
    if (divisor >= 0x80000000U) {
        target_clk_divider = UINT32_MAX;
        return;
    }
    divisor /= 2U;
    target_clk_divider = divisor / (CYCLES_PER_CNT * frequency);
    if (target_clk_divider * (CYCLES_PER_CNT * frequency) < divisor)
        ++target_clk_divider;
}

uint32_t platform_max_frequency_get(void)
{
    return probe_get_swclk_freq() * 1000u;
}
