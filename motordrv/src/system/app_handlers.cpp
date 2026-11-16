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
#include <mcu.h>
#include "system_handlers.h"

static int systick_cnt_ = 0;
static int sub_sec_cnt_ = 0;
static int sec_cnt_ = 0;

extern "C" int getEpoch() { return systick_cnt_; }
extern "C" int getSecCnt() { return sec_cnt_; }

extern "C" void __attribute__((section(".hard_reset"))) 
SysTick_Handler() {
    SysTick_registers_type *systick = (SysTick_registers_type *)SysTick_BASE;

    read32(&systick->CSR);     // Clear [16] COUNTFLAG
    ++systick_cnt_;
    if (++sub_sec_cnt_ >= 100) {
        sub_sec_cnt_ = 0;
        sec_cnt_++;
    }
}
