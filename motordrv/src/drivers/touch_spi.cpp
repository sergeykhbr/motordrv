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

#include "touch_spi.h"
#include <uart.h>
#include <spi.h>
#include <app_common.h>
#include <app_handlers.h>

TouchSPI::TouchSPI(const char *name) : FwObject(name),
    irqstate_(this, "irqstate"),
    posX_(this, "posX", 0x90),
    posY_(this, "posY", 0xD0) {
    RCC_registers_type *RCC = (RCC_registers_type *)RCC_BASE;
    SPI_registers_type *SPI = (SPI_registers_type *)SPI2_BASE;

    gpio_pin_as_input(&TOUCH_IRQ, GPIO_SLOW, GPIO_NO_PUSH_PULL);
    gpio_pin_as_output(&TOUCH_CS,
                       GPIO_NO_OPEN_DRAIN,
                       GPIO_FAST,
                       GPIO_NO_PUSH_PULL);
    gpio_pin_set(&TOUCH_CS);

    // SPI2 (AF5):
    gpio_pin_as_alternate(&TOUCH_SCK, 5);
    gpio_pin_as_alternate(&TOUCH_SDI, 5);
    gpio_pin_as_alternate(&TOUCH_SDO, 5);

    // adc clock on APB2 = 100 MHz
    uint32_t t1 = read32(&RCC->APB1LENR);
    t1 |= (1 << 14);            // APB1L[14] SPI2EN
    write32(&RCC->APB1LENR, t1);

    // Disable SPI before configuring
    write32(&SPI->CR1, 0);

    // SPI master, transmit-only (max 2.5 MHz for touch controller)
    t1 = (5 << 28)      // [30:28] MBR[2:0] master baud rate: 2=1/8; 5=1/64
       | (0 << 22)      // [22] CRCEN: hardware CRC computation disabled
       | (0 << 16)      // [20:16] CRCSIZE[4:0] length of CRC frame
       | (0 << 15)      // [15] TXDMAEN TxDMA stream disabled
       | (0 << 14)      // [14] RXDMAEN RxDMA stream disabled
       | (0 << 11)      // [12:11] UDRDET[1:0] detection of underrun condition at slave transmitter
       | (0 << 9)       // [10:9] UDRCFG[1:0] behavior of slave transmitter at underrun condition
       | (0 << 5)       // [8:5] FTHLV[3:0] FIFO threshold level: 0000=1-data.
       | (7 << 0);      // [4:0] DSIZE[4:0] number of bits in at single SPI data frame: 7=8bits
    write32(&SPI->CFG1, t1);

    t1 = (1 << 31)      // [31] AFCNTR: alterante function GPIOs control:1=keep always controlled by SPI
       | (0 << 30)      // [30] SSOM: SS output management in master mode: 1=SPI data frames are
                        //      interleaved with SS non active pulses when MIDI[3:0] > 1
       | (1 << 29)      // [29] SSOE SS output enable (taken into account only master mode)
       | (0 << 28)      // [28] SSOIP: SS output polarity: 0=LOW active
       | (1 << 26)      // [26] SSM: sw management of SS signal input
       | (0 << 25)      // [25] CPOL: clock polarity: 1=SCK high when idle
       | (0 << 24)      // [24] CPHA: clock phase: 1: the second clock transition is the first data capture edge
       | (0 << 23)      // [23] LSBFST: data frame format: 0=MSB transmitted first
       | (1 << 22)      // [22] MASTER: 1=SPI master
       | (0 << 19)      // [21:19] SP[2:0]: 000=SPI motorola
       | (0 << 17)      // [18:17] COMM[1:0]: SPI communication mode: 00=full duplex
       | (0 << 15)      // [15] IOSWP: swap funcitonality of MISO and MOSI
       | (1 << 4)       // [7:4] MIDI[3:0] master inter-data idleness: 1=1 clock cycle preiod delay
       | (0 << 0);      // [3:0] MSSI[3:0] master SS idleness: 0=no extra delay
    write32(&SPI->CFG2, t1);

    t1 = (0 << 15)      // [31:0] TSER[15:0] number of data transfer extension to be reload into TSIZE
       | (1 << 0);      // [15:0] TSIZE[15:0] number of data at current transfer
    write32(&SPI->CR2, t1);

    t1 = (0 << 16)    // [16] IOLOCK: 0=AF configuration is not locked
       | (0 << 15)    // [15] TCRCINI: CRC calculation initialization pattern control for transmitter
       | (0 << 14)    // [14] RCRCINI: CRC calculation initialization pattern control for receiver
       | (0 << 13)    // [13] CRC33_17: 32-bit CRC polynomial confguration
       | (1 << 12)    // [12] SSI: Internal SS signal input level
       | (1 << 11)    // [11] HDDIR: Rx/Tx direction at half-duplex (ignored in full duplex): 1=SPI is transmitter
       | (0 << 10)    // [10] CSUSP: master suspend request
       | (0 << 9)     // [9] CSTART: master transfer start
       | (0 << 8)     // [8] MASRX: master autnomatic SUSP in Receive mode
       | (0 << 0);    // [0] SPE: serial peripheral enable
    write32(&SPI->CR1, t1);
}

void TouchSPI::Init() {
    RegisterAttribute(&irqstate_);
    RegisterAttribute(&posX_);
    RegisterAttribute(&posY_);
}

void TouchSPI::PostInit() {
}

uint8_t TouchSPI::write_byte(uint8_t byte) {
    SPI_registers_type *SPI = (SPI_registers_type *)SPI2_BASE;
    uint32_t t1;
    uint32_t cr1;
    uint8_t rdbyte = 0;

    write32(&SPI->CR2, 1);  // [15:0] TSIZE = 1 byte
    cr1 = read32(&SPI->CR1);
    cr1 |= 1 << 0;          // [0] SPE: SPI enable
    write32(&SPI->CR1, cr1);

    write8(SPI->TXDR, byte);
    cr1 |= 1 << 9;          // [9] CSTART: master transfer start
    write32(&SPI->CR1, cr1);

    // SR[0] RXP: Rx-packet available (RXP is not set if EOT is set
    // SR[3] EOT: end of transfer
    // SR[12] TXC: TxFIFO tranmission complete
    while ((read32(&SPI->SR) & 0x8) == 0) {}
    // [3] EOTC: end of transfer clear flag
    write32(&SPI->IFCR, ~0ul);
    // SR[0] RXP: Rx-packet available
    rdbyte = read32(&SPI->RXDR);

    cr1 &= ~(1 << 0);       // [0] SPE: SPI disable
    write32(&SPI->CR1, cr1);
    return rdbyte;
}

uint16_t TouchSPI::write_cmd_data(uint8_t cmd, uint8_t *data, int sz) {
    uint16_t ret = 0;
    gpio_pin_clear(&TOUCH_CS);
    write_byte(cmd);
    for (int i = 0; i < sz; i++) {
        ret = (ret << 8) | write_byte(data[i]);
    }
    gpio_pin_set(&TOUCH_CS);
    return ret;
}

void TouchSPI::IrqPinAttribute::pre_read() {
    u_.u32 = gpio_pin_get(&TOUCH_IRQ);
}

void TouchSPI::CoordinateAttribute::pre_read() {
    // 0x90 Y
    // 0xD0 X
    // [7] S Start bit. Control byte starts with HIGH bit on DIN.
    // [6:4] A[2:0] Channel select bits. Alog with SER/DFR bit, these bits cotnrol the setting of the multiplexer input
    // [3] MODE. 0=12-bits; 1=8-bits
    // [2] SER/nDFR Single-neded/differential reference select bit.
    // [1:0] PD[1:0] Power-down mode select bits
    //      00 Power down between Conversion. PENIRQ enabled.
    //      01 Reference is off and ADC is on. PENIRQ disabled.
    //      10 Reference is on and ADC is off. PENIRQ enabled.
    //      11 Device is always powered. Refrence is on and ADC is on. PENIRQ disabled.

    // 12-bit mode
    uint16_t t1 = cmd_;
    t1 |= (1 << 2)  // differental ref clock
       | (1 << 3);  // 8 bit mode
    u_.i32 = 0;
    u_.u16 = parent_->write_cmd_data(t1, &u_.u8, 2) >> 4; // 12 bits ADC
}
