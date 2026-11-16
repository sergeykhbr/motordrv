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

#include <prjtypes.h>
#include <stdio.h>
#include <string.h>
#include <stm32f4xx_map.h>
#include <fwapi.h>
#include <gpio_drv.h>
#include <app_common.h>

static const gpio_pin_type RELAIS1 = {(GPIO_registers_type *)GPIOD_BASE, 3};     // PD3
static const gpio_pin_type RELAIS2 = {(GPIO_registers_type *)GPIOD_BASE, 2};     // PD2

extern "C" int epoch2sec(int epoch) { return epoch / 100; }

static void init_systick() {
    SysTick_registers_type *systick = (SysTick_registers_type *)SysTick_BASE;
    uint32_t t1;

    // SysClock at 72 MHz
    t1 = system_clock_hz() / 2 / 100;   // 100 Hz, SysTick is 24-bits width
    write32(&systick->RVR, t1);         // reload value
    write32(&systick->CVR, t1);         // current value

    t1 = (1 << 2)      // CLKSOURCE: SysTick uses CPU clock (default 1)
       | (1 << 1)       // TICKINT: enable interrupt
       | 1;            // ENABLE:
    write32(&systick->CSR, t1);
}

extern "C" int fwmain(int argcnt, char *args[]) {
    init_systick();
    EnableIrqGlobal();

    fw_init();

    gpio_pin_as_output(&RELAIS1,
                       GPIO_NO_OPEN_DRAIN,
                       GPIO_FAST,
                       GPIO_NO_PUSH_PULL);
    gpio_pin_as_output(&RELAIS2,
                       GPIO_NO_OPEN_DRAIN,
                       GPIO_FAST,
                       GPIO_NO_PUSH_PULL);

    int tcnt = getSecCnt();
    int flag = 0;
    while(1) {
        if (tcnt != getSecCnt()) {
            tcnt = getSecCnt();
            flag++;
            if (flag & 1) {
                //gpio_pin_set(&RELAIS1);
                gpio_pin_set(&RELAIS2);
            } else {
                //gpio_pin_clear(&RELAIS1);
                gpio_pin_clear(&RELAIS2);
            }
        }
    }

    return 0;
}

