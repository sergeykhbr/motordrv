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

#include <fwapi.h>
#include "app_fwkernel.h"
#include <string.h>
#include <new>

/**
 * @brief Kernel module constructor. Instance of this KernelClass registered
 *        as the first element in the single linked list of FwObjects.
 */
AppKernelClass::AppKernelClass(const char *name) : KernelClassGeneric(name),
    settings_("usrset"),
    disp0_("disp0"),
    relais0_("relais0", 0),
    relais1_("relais1", 1),
    ubtn0_("ubtn0"),
    rtc_("rtc")
{
    version_.make_uint32(0x20240804);
    output_.make_int32(0);
}

extern "C" void kernel_init() {
    new (fw_malloc(sizeof(AppKernelClass))) AppKernelClass("kernel");
}

