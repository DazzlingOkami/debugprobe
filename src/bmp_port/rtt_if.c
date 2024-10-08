/*
 * This file is part of the Black Magic Debug project.
 *
 * MIT License
 *
 * Copyright (c) 2021 Koen De Vleeschauwer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "general.h"
#include "platform.h"
#include <assert.h>
#include "rtt.h"
#include "rtt_if.h"
#include "tusb.h"

/*********************************************************************
*
*       rtt terminal i/o
*
**********************************************************************
*/
#define RTT_USB_PORT (0)

/* rtt host to target: read one character */
int32_t rtt_getchar()
{
#ifdef ENABLE_RTT
    if(tud_cdc_n_available(RTT_USB_PORT)){
        return tud_cdc_n_read_char(RTT_USB_PORT);
    }
#endif
    return -1;
}

/* rtt host to target: true if no characters available for reading */
bool rtt_nodata()
{
#ifdef ENABLE_RTT
    return tud_cdc_n_available(RTT_USB_PORT) == 0;
#else
    return true;
#endif
}

/* rtt target to host: write string */
uint32_t rtt_write(const char *buf, uint32_t len)
{
#ifdef ENABLE_RTT
    if(tud_cdc_n_connected(RTT_USB_PORT) == false){
        tud_cdc_n_write_clear(RTT_USB_PORT);
        return 0;
    }
    uint32_t xfer = 0;
    while(xfer < len){
        uint32_t chunk = tud_cdc_n_write(RTT_USB_PORT, buf + xfer, len - xfer);
        xfer += chunk;

        if(chunk == 0){
            platform_delay(1);
            if(tud_cdc_n_connected(RTT_USB_PORT) == false){
                tud_cdc_n_write_clear(RTT_USB_PORT);
                break;
            }
        }
    }
    return xfer;
#endif
}

void debug_serial_send_stdout(const uint8_t *const data, const size_t len){
    if(tud_cdc_n_connected(RTT_USB_PORT) == false){
        tud_cdc_n_write_clear(RTT_USB_PORT);
        return ;
    }
    uint32_t xfer = 0;
    while(xfer < len){
        uint32_t chunk = tud_cdc_n_write(RTT_USB_PORT, data + xfer, len - xfer);
        xfer += chunk;

        if(chunk == 0){
            platform_delay(1);
            if(tud_cdc_n_connected(RTT_USB_PORT) == false){
                tud_cdc_n_write_clear(RTT_USB_PORT);
                break;
            }
        }
    }
    return ;
}
