/*
 *  Copyright 2025 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <stdarg.h>
#include "CommonInterface.h"

static const int HEIGHT    = 240;
static const int WIDTH     = 320;

static const uint16_t CLR_BLACK = 0x0000;
static const uint16_t CLR_WHITE = 0xFFFF;
static const uint16_t CLR_TRNSPARENT = 0x0020;
// grey
static const uint16_t CLR_GRAY = 0x8410;
static const uint16_t CLR_STATEGRAY = 0x7412;
static const uint16_t CLR_DARKGRAY = 0x0841;
static const uint16_t CLR_DIMGRAY = 0xdb4d;
// red tones
static const uint16_t CLR_RED = 0xF800;
static const uint16_t CLR_ORANGERED = 0xFA20;
static const uint16_t CLR_ORANGE = 0xFD20;
static const uint16_t CLR_TOMATO = 0xfb09;
static const uint16_t CLR_CRIMSON = 0xd8a7;
// green tones
static const uint16_t CLR_GREEN = 0x07E0;
static const uint16_t CLR_LIGHTGREEN = 0x9772;
static const uint16_t CLR_OLIVE  = 0x8400;
static const uint16_t CLR_PALEGREEN  = 0x97d2;
// blue tones
static const uint16_t CLR_BLUE  = 0x003F;
static const uint16_t CLR_SKYBLUE  = 0x867d;
static const uint16_t CLR_STATEBLUE = 0x6ad9;
// purple tones
static const uint16_t CLR_PURPLE  = 0x8010;
static const uint16_t CLR_ORCHID  = 0xdb9a;
static const uint16_t CLR_PLUM    = 0xdd1b;
static const uint16_t CLR_DARKVIOLET = 0x3009;
// yellow
static const uint16_t CLR_YELLOW  = 0xffe0;
static const uint16_t CLR_YELLOWGREEN = 0x9e66;
static const uint16_t CLR_GOLD  = 0xfea0;

class DisplayInterface : public CommonInterface {
 public:
    DisplayInterface() : CommonInterface("DisplayInterface") {}

    virtual void clearRegion(int h1, int w1, int h2, int w2, uint16_t clr) = 0;
    // Font Size 24, total 8 lines, 19 symbols per line
    virtual void outputText24Line(const char *str, int linepos, int symbpos, uint32_t clr, uint32_t bkgclr) = 0;
    virtual void outputText16Line(const char *str, int linepos, int symbpos, uint32_t clr, uint32_t bkgclr) = 0;
};
