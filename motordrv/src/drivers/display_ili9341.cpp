/*
 *  Copyright 2024 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <prjtypes.h>
#include <uart.h>
#include <spi.h>
#include <app_common.h>
#include "display_spi.h"

void DisplaySPI::init_ili9341() {
    SPI_registers_type *SPI = (SPI_registers_type *)SPI3_BASE;
    uint32_t t1;
    uint8_t tbuf[16];

    //uart_printk("LCD: resetting...\r\n");
    gpio_pin_clear(&DISPLAY_RES);   // reset: active LOW
    t1 = getEpoch();
    while ((t1 + 20) > getEpoch()) {}   // ~200 ms

    gpio_pin_set(&DISPLAY_RES);
    t1 = getEpoch();
    while ((t1 + 20) > getEpoch()) {}   // ~200ms
    //uart_printk("LCD: reset done\r\n");

    
    write_cmd_data(0x01, 0, 0);           // 0x01 soft reset command
    t1 = getEpoch();
    while ((t1 + 100) > getEpoch()) {}   // ~1000 ms
    
    //power control A (8.4.1 exteneded register)
    tbuf[0] = 0x39;
    tbuf[1] = 0x2C;
    tbuf[2] = 0x00;
    tbuf[3] = 0x34;     // VCORE=1.6V
    tbuf[4] = 0x02;     // DDVDH=5.6V
    write_cmd_data(0xCB, tbuf, 5);

    //power control B
    tbuf[0] = 0x00;
    tbuf[1] = 0xC1;     // [6] PCEQ: 0=disable PC and EQ for power saving
    tbuf[2] = 0x30;
    write_cmd_data(0xCF, tbuf, 3);

    //driver timing control A
    tbuf[0] = 0x85;
    tbuf[1] = 0x10;//0x00;
    tbuf[2] = 0x78;
    write_cmd_data(0xE8, tbuf, 3);

    //driver timing control B
    tbuf[0] = 0x00;
    tbuf[1] = 0x00;
    write_cmd_data(0xEA, tbuf, 2);

    //power on sequence control
    tbuf[0] = 0x64;
    tbuf[1] = 0x03;
    tbuf[2] = 0x12;
    tbuf[3] = 0x81;
    write_cmd_data(0xED, tbuf, 4);

	//pump ratio control
    tbuf[0] = 0x20;
    write_cmd_data(0xF7, tbuf, 1);

	//power control,VRH[5:0]
    tbuf[0] = 0x21;//0x23;
    write_cmd_data(0xC0, tbuf, 1);

	//Power control,SAP[2:0];BT[3:0]
    tbuf[0] = 0x12;//0x10;
    write_cmd_data(0xC1, tbuf, 1);

	//vcm control
    tbuf[0] = 0x40;//0x3E;
    tbuf[1] = 0x3C;//0x28;
    write_cmd_data(0xC5, tbuf, 2);

	//vcm control 2
    tbuf[0] = 0x94;//0x86;
    write_cmd_data(0xC7, tbuf, 1);

    // To rotate 180:
    //    MY=1,MX=0 = default CAN connectors down, buttons on right side
    //    MY=0,MX=1 = rotated CAN connectors up, buttons on left side
    //memory access control
    // [7] MY - row address order
    // [6] MX - Column address order
    // [5] MV - Row/column exchange
    // [4] ML - Vertical refresh order
    // [3] BGR - RGB/BGR order: 0=RGB; 1=BGR
    // [2] MH - Horizontal refresh order
    // [1:0] = 00
#if defined(DISPLAY_24INCH)
    // new display with working touchscreen
    tbuf[0] = 0x48;
#elif defined(DISPLAY_28INCH)    
    tbuf[0] = 0x80;
#else
    // 2.4' no touch
    tbuf[0] = 0x80;
#endif
    write_cmd_data(0x36, tbuf, 1);

    //pixel format
    tbuf[0] = 0x55;
    write_cmd_data(0x3A, tbuf, 1);

    // Frame Rate Control, normal mode full colours
    tbuf[0] = 0x00;     // default (0x00)
    tbuf[1] = 0x17;     // 0x17=83Hz; 0x18=79Hz; default (0x1B = 70Hz)
    write_cmd_data(0xB1, tbuf, 2);

    // Display Function Control
    tbuf[0] = 0x0A;     // [3:2] PTG: Scan mode in non-display area
                        // [1:0] PT: Determine source/VCOM output in a non-disaply
    tbuf[1] = 0xA2;     // [7] REV: liquide crystal type: 0=normally black; 1=white
                        // [6] GS: Direction of scan by the gate driver
                        // [5] SS: Shift direction of output from source driver
                        // [4] SM: The gate driver pin arragement in combination with GS
                        // [3:0] ISC: Scan cycle interval: 0010=5 frames=85 ms
    tbuf[2] = 0x27;     // Number of line to drive: 0x27 = 320 lines
    write_cmd_data(0xB6, tbuf, 3);

    // Interface control:
    tbuf[0] = 0x01; // [0] WEMODE=1; [3] BGR_FOR; [5] MV_EOR; [6] MX_EOR; [7] MY_EOR
    tbuf[1] = 0x10; // [5:4] EPF[1:0]=01; [1:0] MDT=00
    tbuf[2] = 0x00; // [5] ENDIAN; [3:2] DM; [1] RM; [0] RIM
    write_cmd_data(0xF6, tbuf, 3);

    //3gamma function disable
    tbuf[0] = 0x00;
    write_cmd_data(0xF2, tbuf, 1);

    //gamma curve selected
    tbuf[0] = 0x01;
    write_cmd_data(0x26, tbuf, 1);

    //exit sleep
    write_cmd_data(0x11, 0, 0);
    t1 = getEpoch();
    while ((t1 + 13) > getEpoch()) {}   // ~130 ms
	
    //display on
    write_cmd_data(0x29, 0, 0);

    clearRegion(0, 0, HEIGHT, WIDTH, CLR_GOLD); 		// clear full screen
    outputText24Line("Hi!", 50, 60, CLR_BLUE, CLR_BLACK);
}

