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
