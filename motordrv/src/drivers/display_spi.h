/*
 *  Copyright 2026 Sergey Khabarov, sergeykhbr@gmail.com
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
#pragma once

#include <gardemarin.h>
#include <prjtypes.h>
#include <fwlist.h>
#include <fwobject.h>
#include <FwAttribute.h>
#include <SensorInterface.h>
#include <IrqInterface.h>
#include <DisplayInterface.h>
#include <mcu.h>
#include <gpio_drv.h>

// DISPLAY -> SPI3 (AF5)
static const gpio_pin_type DISPLAY_SDO = {(GPIO_registers_type *)GPIOC_BASE, 11}; // SPI3_MISO (79 PC11)
static const gpio_pin_type DISPLAY_SCK = {(GPIO_registers_type *)GPIOC_BASE, 10}; // SPI3_SCK (78 PC10)
static const gpio_pin_type DISPLAY_SDI = {(GPIO_registers_type *)GPIOC_BASE, 12}; // SPI3_MOSI (80 PC12)
static const gpio_pin_type DISPLAY_DC = {(GPIO_registers_type *)GPIOA_BASE, 11};  // PA11_DISP_DC
static const gpio_pin_type DISPLAY_RES = {(GPIO_registers_type *)GPIOA_BASE, 12}; // PA12_DISP_RESET
static const gpio_pin_type DISPLAY_CS = {(GPIO_registers_type *)GPIOE_BASE, 1};   // PE1_DISP_CS
static const gpio_pin_type TOUCH_CS = {(GPIO_registers_type *)GPIOA_BASE, 8};     // PA8_TOUCH_CS


class DisplaySPI : public FwObject,
                   public IrqHandlerInterface,
                   public DisplayInterface {
 public:
    DisplaySPI(const char *name);

    // FwObject interface:
    virtual void Init() override;
    virtual void PostInit() override;

    // IrqHandlerInterface: SPI tx is empty
    virtual void handleInterrupt() override;

    // IDisplayInterface
    virtual void clearRegion(int x1, int y1, int x2, int y2, uint16_t clr);
    virtual void drawRegion(int x1, int y1, int x2, int y2, uint16_t *buf);
    virtual void outputText24Line(const char *str, int linepos, int symbpos, uint32_t clr, uint32_t bkgclr);
    virtual void outputText16Line(const char *str, int linepos, int symbpos, uint32_t clr, uint32_t bkgclr);

 protected:
    virtual void outputTextLine(const uint8_t *arr, uint8_t fontsz,
                                const char *str, int linepos,
                                int symbpos,
                                uint32_t clr, uint32_t bkgclr);

    virtual void write_cmd_data(uint8_t cmd, uint8_t *data, int sz);
    virtual void write_cmd_poll(uint8_t cmd);
    virtual uint8_t write_data_poll(uint8_t data);
    virtual uint8_t write_byte(uint8_t byte);

 protected:

    virtual void init_ili9341();
    virtual void set_address(uint16_t xstart, uint16_t xend,
                           uint16_t ystart, uint16_t yend);
    virtual void symb2buff(const uint8_t *chbuf,
                           int row,
                           int col,
                           int w,
                           int h,
                           uint16_t clr,
                           uint16_t bkg);

    uint16_t fb_[24 * 12];   // one symbol
};

