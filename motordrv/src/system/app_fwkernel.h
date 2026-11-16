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

#pragma once

#include <fwkernelgen.h>
#include "display_spi.h"
#include "relais.h"
#include "ledstrip.h"
#include "user_btn.h"
#include "usrsettings.h"

class AppKernelClass : public KernelClassGeneric {
 public:
    explicit AppKernelClass(const char *name);

    // FwObject interface:
    //virtual void PostInit() override;

 private:
    /** @brief Kernel Version attribute */
    UserSettings settings_;
    DisplaySPI disp0_;     // let make display first to make available on postinit and irq of others modules
    RelaisDriver relais0_;
    RelaisDriver relais1_;
    UserButtonDriver ubtn0_;
    RtcDriver rtc_;
};
