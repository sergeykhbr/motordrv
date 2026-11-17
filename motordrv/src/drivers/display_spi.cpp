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

#include "display_spi.h"
#include "display_fonts.h"
#include <new>
#include <string.h>
#include <spi.h>
#include <app_common.h>

DisplaySPI::DisplaySPI(const char *name) : FwObject(name) {
    RCC_registers_type *RCC = (RCC_registers_type *)RCC_BASE;
    SPI_registers_type *SPI = (SPI_registers_type *)SPI3_BASE;

    // Setup SPI interface. Use bitband instead of SPI controller because 
    // non-standard 25-bits transactions:
    gpio_pin_as_output(&DISPLAY_RES,
                       GPIO_NO_OPEN_DRAIN,
                       GPIO_MEDIUM,
                       GPIO_NO_PUSH_PULL);
    gpio_pin_set(&DISPLAY_RES);   // reset: active LOW

    gpio_pin_as_output(&DISPLAY_DC,
                       GPIO_NO_OPEN_DRAIN,
                       GPIO_FAST,
                       GPIO_NO_PUSH_PULL);
    gpio_pin_set(&DISPLAY_DC);

    gpio_pin_as_output(&DISPLAY_CS,
                       GPIO_NO_OPEN_DRAIN,
                       GPIO_FAST,
                       GPIO_NO_PUSH_PULL);
    gpio_pin_set(&DISPLAY_CS);

    gpio_pin_as_output(&TOUCH_CS,
                       GPIO_NO_OPEN_DRAIN,
                       GPIO_FAST,
                       GPIO_NO_PUSH_PULL);
    gpio_pin_set(&TOUCH_CS);

    // SPI3 (AF6):
    gpio_pin_as_alternate(&DISPLAY_SCK, 6);
    gpio_pin_as_alternate(&DISPLAY_SDI, 6);
    gpio_pin_as_alternate(&DISPLAY_SDO, 6);

    // SPI3 clock on APB1 = 144/4 = 36 MHz
    uint32_t t1 = read32(&RCC->APB1ENR);
    t1 |= (1 << 15);            // APB1[15] SPI3EN
    write32(&RCC->APB1ENR, t1);

    // Disable SPI before configuring
    write16(&SPI->CR1, 0);

    // SPI3 master, transmit-only
    uint16_t t2;
    t2 = (0 << 7)    // [7] TXEIE: Tx buffer empty IRQ
       | (0 << 6)    // [6] RXNEIE: Rx buffer not empty IRQ
       | (0 << 5)    // [5] ERRIE: Error IRQ
       | (0 << 4)    // [4] FRF: Frame format: 0=SPI motorola; 1=SPI TI
       | (0 << 2)    // [2] SSOE: SS output enable: 0=disabled (can work multimaster mode)
       | (0 << 1)    // [1] TXDMAEN: Tx buffer DMA ena
       | (0 << 0);   // [0] RXDMAEN: Rx buffer DMA ena
    write16(&SPI->CR2, t2);

    // baudrate 36MHz / 32 = ~1MHz
    t2 = (0 << 15)    // [15] BIDIMODE: 0=2-line unidirectional, 1=1-linne bidirectional
       | (1 << 14)    // [14] BIDIOE: output enable in bidir mode
       | (0 << 13)    // [13] CRCEN: hardware CRC calc ena
       | (0 << 12)    // [12] CRCNEXT: CRC transfer next
       | (0 << 11)    // [11] DFF: Data frame format: 0=8bits, 1=16bits
       | (0 << 10)    // [10] RXONLY: Receive only
       | (1 << 9)     // [9] SSM: Software Slave Management. When 1(enabled) the NSS pin input is replaced with the value from SSI bit
       | (1 << 8)     // [8] SSI: Internal Slave select
       | (0 << 7)     // [7] LSBFIRST: 0=MSB tranmit first, 1=LSB
       | (0 << 6)     // [6] SPE: SPI enable
       | (4 << 3)     // [5:3] BR[2:0]: Baud rate control: 0-Fpclk/2, 1=Fpclk/4, 2=Fpclk/8, ...,4=Fpclk/32, ..,  7=Fpclk/256
       | (1 << 2)     // [2] MSTR: Master selection
       | (1 << 1)     // [1] CPOL: Clock polarity: 0=CK to 0 when idle, 1=when idle
       | (1 << 0);    // [0] CPHA: Clock phase: 0=the first clock transition is the first data capture edge
    write16(&SPI->CR1, t2);

    t2 |= (1 << 6);     // [6] SPE: SPI enable
    write16(&SPI->CR1, t2);

    // prio: 0 highest; 7 is lowest
    //nvic_irq_enable(51, 6); // 51 SPI3
}

void DisplaySPI::Init() {
    RegisterInterface(static_cast<IrqHandlerInterface *>(this));
    RegisterInterface(static_cast<DisplayInterface *>(this));
}

void DisplaySPI::PostInit() {
    memset(fb_, 0x0, sizeof(fb_));
    init_ili9341();
}

uint8_t DisplaySPI::write_byte(uint8_t byte) {
    SPI_registers_type *SPI = (SPI_registers_type *)SPI3_BASE;
    gpio_pin_clear(&DISPLAY_CS);
    write16(&SPI->DR, byte);
    // SR[1] TXE: transmit buffer empty: 0=not empty; 1=empty
    // SR[7] BSY: Busy flag: 1 Tx buffer is not empty
    while ((read16(&SPI->SR) & 0x2) == 0) {}
    while ((read16(&SPI->SR) & 0x80)) {}
    gpio_pin_set(&DISPLAY_CS);
    return read16(&SPI->DR);
}

void DisplaySPI::write_cmd_data(uint8_t cmd, uint8_t *data, int sz) {
    gpio_pin_clear(&DISPLAY_DC);    // 0=CMD
    write_byte(cmd);

    gpio_pin_set(&DISPLAY_DC);    // 1=DATA
    for (int i = 0; i < sz; i++) {
        write_byte(data[i]);
    }
}

void DisplaySPI::write_cmd_poll(uint8_t cmd) {
    gpio_pin_clear(&DISPLAY_DC);    // 0=CMD
    write_byte(cmd);
    gpio_pin_set(&DISPLAY_DC);    // 1=DATA
}

uint8_t DisplaySPI::write_data_poll(uint8_t data) {
    gpio_pin_set(&DISPLAY_DC);      // 1=DATA
    return write_byte(data);
}

void DisplaySPI::handleInterrupt() {
}

void DisplaySPI::set_address(uint16_t x1, uint16_t y1,
                             uint16_t x2, uint16_t y2) {
    uint8_t tstr[8];
    x2 -= 1;
    tstr[0] = static_cast<uint8_t>(x1 >> 8);
    tstr[1] = static_cast<uint8_t>(x1);
    tstr[2] = static_cast<uint8_t>(x2 >> 8);
    tstr[3] = static_cast<uint8_t>(x2);
    write_cmd_data(0x2A, tstr, 4);

    y2 -= 1;
    tstr[0] = static_cast<uint8_t>(y1 >> 8);
    tstr[1] = static_cast<uint8_t>(y1);
    tstr[2] = static_cast<uint8_t>(y2 >> 8);
    tstr[3] = static_cast<uint8_t>(y2);
    write_cmd_data(0x2B, tstr, 4);

    // prepare to memory write
    write_cmd_data(0x2C, 0, 0);
}

void DisplaySPI::clearRegion(int h1, int w1, int h2, int w2, uint16_t clr) {
    int Nh = h2 - h1;
    int Nw = w2 - w1;

    for (int y = 0; y < Nw; y++) {
        set_address(h1, w1, h2, w1 + 1);
        for (int x = 0; x < Nh; x++) {
            write_data_poll(static_cast<uint8_t>(clr >> 8));
            write_data_poll(static_cast<uint8_t>(clr));
        }
        w1++;
    }
}

void DisplaySPI::drawRegion(int h1, int w1, int h2, int w2, uint16_t *buf) {
    uint8_t *b8 = reinterpret_cast<uint8_t*>(buf);
    int Nh = h2 - h1;
    int Nw = w2 - w1;

    for (int y = 0; y < Nw; y++) {
        set_address(h1, w1, h2, w1 + 1);
        for (int x = 0; x < Nh; x++) {
            write_data_poll(*b8++);
            write_data_poll(*b8++);
        }
        w1++;
    }
}

void DisplaySPI::symb2buff(const uint8_t *chbuf,
                           int h1,
                           int w1,
                           int h,
                           int w,
                           uint16_t clr,
                           uint16_t bkg)
{
    uint16_t *buf16 = &fb_[0];
    uint8_t symb;
    for (int y = 0; y < w; y++) {
        for (int x = 0; x < h; x++) {
            if ((x & 0x7) == 0) {
                symb = *chbuf++;
            }
            if (symb & 0x80) {
                *buf16 = clr;
            } else {
                *buf16 = bkg;
            }
            symb <<= 1;
            buf16++;
        }
        //buf16 += (HEIGHT - h);
    }
}

void DisplaySPI::outputTextLine(const uint8_t *arr,
                                uint8_t fontsz,
                                const char *str,
                                int linepos,
                                int symbpos,
                                uint32_t clr,
                                uint32_t bkgclr) {
    const uint8_t *chbuf;
    int w1 = symbpos;
    int w2 = w1 + (fontsz >> 1);
    int h1 = linepos;
    int h2 = h1 + fontsz;

    int bytes_per_symb = 36;
    if (fontsz == 16) {
        bytes_per_symb = 16;
    } else if (fontsz == 12) {
        bytes_per_symb = 12;
    }

    while ((*str <= '~') && (*str >= ' ')) {
        chbuf = &arr[bytes_per_symb * (int)(*str - ' ')];

        symb2buff(chbuf, h1, w1, fontsz, fontsz/2, clr, bkgclr);
        drawRegion(h1, w1, h2, w2, fb_);

        w1 += (uint8_t)(fontsz/2);
        w2 += (uint8_t)(fontsz/2);
        str++;
    }
}

void DisplaySPI::outputText16Line(const char *str, int linepos, int symbpos, uint32_t clr, uint32_t bkgclr) {
    outputTextLine(reinterpret_cast<const uint8_t *>(ascii_1608), 16,
                    str, linepos, symbpos, clr, bkgclr);
}

void DisplaySPI::outputText24Line(const char *str, int linepos, int symbpos, uint32_t clr, uint32_t bkgclr) {
    outputTextLine(reinterpret_cast<const uint8_t *>(ascii_2412), 24,
                    str, linepos, symbpos, clr, bkgclr);
}
