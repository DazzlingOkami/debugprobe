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

/*
 * This file implements a transparent channel over which the GDB Remote
 * Serial Debugging protocol is implemented. This implementation for STM32
 * uses the USB CDC-ACM device bulk endpoints to implement the channel.
 */

#include "general.h"
#include "platform.h"
#include "gdb_if.h"
#include "tusb.h"

#define GDB_USB_PORT (1)

void gdb_if_putchar(const char c, const int flush)
{
    if(tud_cdc_n_connected(GDB_USB_PORT) == false){
        tud_cdc_n_write_clear(GDB_USB_PORT);
        return ;
    }
    while(tud_cdc_n_write_available(GDB_USB_PORT) == 0){
        platform_delay(1);
        if(tud_cdc_n_connected(GDB_USB_PORT) == false){
            tud_cdc_n_write_clear(GDB_USB_PORT);
            return ;
        }
    }
    tud_cdc_n_write_char(GDB_USB_PORT, c);
    if(flush){
        tud_cdc_n_write_flush(GDB_USB_PORT);
    }
}

char gdb_if_getchar(void)
{
    if(tud_cdc_n_connected(GDB_USB_PORT) == false){
        platform_delay(10);
        return '\x04';
    }

    while(tud_cdc_n_available(GDB_USB_PORT) == 0){
        platform_delay(1);
        if(tud_cdc_n_connected(GDB_USB_PORT) == false){
            return '\x04';
        }
    }

    return tud_cdc_n_read_char(GDB_USB_PORT);
}

char gdb_if_getchar_to(const uint32_t timeout)
{
    for(int i = 0; i < timeout && tud_cdc_n_available(GDB_USB_PORT) == 0; i+=5){
        if(tud_cdc_n_connected(GDB_USB_PORT) == false){
            return '\x04';
        }
        platform_delay(1);
    }

    if(tud_cdc_n_available(GDB_USB_PORT) == 0){
        return -1;
    }

    return tud_cdc_n_read_char(GDB_USB_PORT);
}
