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

#include <inttypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

//#define DISPLAY_28INCH
#ifdef DISPLAY_28INCH
    // TODO: through calibration non-volalite rate
    #define DISPLAY_28INCH_WHITE_BLUE
#endif
#define DISPLAY_24INCH
//#define DISPLAY_24INCH_NOTOUCH

#define BYTESWAP16(x) ((uint16_t)((x << 8) | ((x >> 8) & 0xFF)))
#define BYTESWAP32(x) (((x << 24) | ((x << 8) & 0xFF0000) | ((x >> 8) & 0xFF00) | ((x >> 24) & 0xFF)))

int getEpoch();
int getSecCnt();
int epoch2sec(int epoch);

#ifdef __cplusplus
}
#endif

