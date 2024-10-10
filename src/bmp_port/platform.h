/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
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
