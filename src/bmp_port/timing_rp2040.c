/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2015 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2023 1BitSquared <info@1bitsquared.com>
 * Modified by Rachel Mant <git@dragonmux.network>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "general.h"
#include "platform.h"
#include "morse.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define rcc_ahb_frequency (125 * 1000 * 1000)

bool running_status = false;
uint32_t target_clk_divider = 0;

static void morse_timer_callback(TimerHandle_t ptmr){
    if (running_status)
        gpio_toggle(LED_PORT, LED_IDLE_RUN);
    SET_ERROR_STATE(morse_update());
}

void platform_timing_init(void)
{
    TimerHandle_t morse_tmr;
    morse_tmr = xTimerCreate("morse", pdMS_TO_TICKS(MORSECNT), 
        1, NULL, morse_timer_callback);
    xTimerStart(morse_tmr, 0xffff);
}

void platform_delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

uint32_t platform_time_ms(void)
{
    return xTaskGetTickCount() * 1000u / configTICK_RATE_HZ;
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
    #if 0
    uint32_t result = rcc_ahb_frequency;
    result /= USED_SWD_CYCLES + CYCLES_PER_CNT * target_clk_divider * 2;
    return result;
    #else
    return probe_get_swclk_freq() * 1000u;
    #endif
}
