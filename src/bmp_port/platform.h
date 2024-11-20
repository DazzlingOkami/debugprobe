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

/* This file provides the platform specific declarations for the pico-rp2040 implementation. */

#ifndef PLATFORMS_RP2040_PLATFORM_H
#define PLATFORMS_RP2040_PLATFORM_H

#include "timing.h"
#include "timing_rp2040.h"
#include <pico/stdlib.h>
#include "probe.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define PLATFORM_IDENT      "(rp2040) "
#define PLATFORM_HAS_POWER_SWITCH

/*
 * Important pin mappings for rp2040 implementation:
 *   * JTAG/SWD(Not used)
 *     * GPIO25: TDI
 *     * GPIO25: TDO/TRACESWO
 *     * GPIO25: TCK
 *     * GPIO25: TMS
 *   * SWD
 *     * Drive by PIO
 *   * +3V3
 *     * GPIO11: power pin
 *     * GPIO26: power adc
 */

/* Hardware definitions... */
#define TDI_PORT    NULL
#define TDI_PIN     25

#define TDO_PORT    NULL
#define TDO_PIN     25

#define TCK_PORT    NULL
#define TCK_PIN     25
#define SWCLK_PORT TCK_PORT
#define SWCLK_PIN  TCK_PIN

#define TMS_PORT    NULL
#define TMS_PIN     25
#define SWDIO_PORT TMS_PORT
#define SWDIO_PIN  TMS_PIN

#define TMS_SET_MODE() 

#define PWR_BR_PORT     NULL
#define PWR_BR_PIN      11

#define LED_PORT        NULL
#define LED_IDLE_RUN    23 // TODO
#define LED_ERROR       24

#define SET_RUN_STATE(state)      \
    {                             \
        running_status = (state); \
    }

#define SET_IDLE_STATE(state)                       \
    {                                               \
        gpio_set_val(LED_PORT, LED_IDLE_RUN, !state);\
    }

#define SET_ERROR_STATE(state)                    \
    {                                             \
        gpio_set_val(LED_PORT, LED_ERROR, state); \
    }

#define gpio_set(g, p)          gpio_put(p, 1)
#define gpio_clear(g, p)        gpio_put(p, 0)
#define gpio_read(g, p)         gpio_get(p)
#define gpio_set_val(g, p, x)   gpio_put(p, (x))
#define gpio_toggle(g, p)       gpio_xor_mask(1 << (p))

#endif /* PLATFORMS_RP2040_PLATFORM_H */
