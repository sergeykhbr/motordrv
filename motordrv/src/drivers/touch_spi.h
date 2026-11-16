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
#include <fwapi.h>
#include <fwobject.h>
#include <FwAttribute.h>
#include <IrqInterface.h>
#include <gpio_drv.h>
#include <mcu.h>

// TOUCH screen -> SPI2 (AF5)
static const gpio_pin_type TOUCH_IRQ = {(GPIO_registers_type *)GPIOB_BASE, 2};
static const gpio_pin_type TOUCH_SDI = {(GPIO_registers_type *)GPIOB_BASE, 14};
static const gpio_pin_type TOUCH_SDO = {(GPIO_registers_type *)GPIOB_BASE, 15};
static const gpio_pin_type TOUCH_CS = {(GPIO_registers_type *)GPIOA_BASE, 11};
static const gpio_pin_type TOUCH_SCK = {(GPIO_registers_type *)GPIOA_BASE, 12};


class TouchSPI : public FwObject {
 public:
    explicit TouchSPI(const char *name);

    // FwObject interface:
    virtual void Init() override;
    virtual void PostInit() override;

 protected:
    class IrqPinAttribute : public FwAttribute {
     public:
        IrqPinAttribute(TouchSPI *parent, const char *name)
            : FwAttribute(name), parent_(parent) {}
        virtual void pre_read() override;
     protected:
        TouchSPI *parent_;
    };

    class CoordinateAttribute : public FwAttribute {
     public:
        CoordinateAttribute(TouchSPI *parent, const char *name, uint8_t cmd)
            : FwAttribute(name), parent_(parent), cmd_(cmd) {}
        virtual void pre_read() override;
     protected:
        TouchSPI *parent_;
        uint8_t cmd_;
    };
 protected:
    virtual uint16_t write_cmd_data(uint8_t cmd, uint8_t *data, int sz);
    virtual uint8_t write_byte(uint8_t byte);

 protected:
    IrqPinAttribute irqstate_;
    CoordinateAttribute posX_;
    CoordinateAttribute posY_;
};
