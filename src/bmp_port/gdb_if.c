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

/*
 * This file implements a transparent channel over which the GDB Remote
 * Serial Debugging protocol is implemented. This implementation for rp2040
 * uses the TinyUSB library to implement a CDC device.
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
