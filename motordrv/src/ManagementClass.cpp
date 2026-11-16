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
#include <uart.h>
#include "ManagementClass.h"


ManagementClass::ManagementClass()
    : FwObject("man") {
    btnClick_ = false;
    epochCnt_ = 0;
    epochMarker_ = 0;
    stateSwitchedLast_ = 0;
}

void ManagementClass::Init() {
}

void ManagementClass::PostInit() {
    KeyInterface *iface = reinterpret_cast<KeyInterface *>(
            fw_get_object_interface("ubtn0", "KeyInterface"));
    if (iface) {
        iface->registerKeyListener(
            static_cast<KeyListenerInterface *>(this));
    }
}


void ManagementClass::update() {
    int btnClick = btnClick_;
    btnClick_ = 0;

    epochCnt_++;

}

void ManagementClass::keyPressed() {
    btnClick_ = true;
}

void ManagementClass::waitKeyPressed() {
}

void ManagementClass::setDayLights(uint32_t tow) {
    uint32_t dayStart = read_uint32("usrset", "DayStart");
    uint32_t dayEnd = read_uint32("usrset", "DayEnd");
    if (tow < dayStart || tow > dayEnd) {
        write_int8("ledrbw", "duty0", 0);   // blue
        write_int8("ledrbw", "duty1", 0);   // ventilation
        write_int8("ledrbw", "duty2", 0);   // white
        write_int8("ledrbw", "duty3", 0);   // red/blue
        disableRelaisLight();
    } else if (tow >= dayStart && tow < (dayStart + 30*60)) {
        // sunrise 30 minutes
        float dt = static_cast<float>(tow - dayStart)/ 1800.0f;
        write_int8("ledrbw", "duty0", 
            static_cast<int8_t>(dt * read_int8("usrset", "DayDuty0")));   // blue
        //write_int8("ledrbw", "duty1",
        //    static_cast<int8_t>(dt * read_int8("usrset", "DayDuty1")));   // unused
        write_int8("ledrbw", "duty2",
            static_cast<int8_t>(dt * 90));   // white
        write_int8("ledrbw", "duty3",
            static_cast<int8_t>(dt * read_int8("usrset", "DayDuty3")));   // red/blue
    } else if (tow >= (dayEnd - 30*60) && tow < dayEnd) {
        // sunset 30 minutes
        float dt = 1.0f - static_cast<float>(tow - (dayEnd - 1800))/ 1800.0f;
        write_int8("ledrbw", "duty0", 
            static_cast<int8_t>(dt * read_int8("usrset", "DayDuty0")));   // blue
        //write_int8("ledrbw", "duty1",
        //    static_cast<int8_t>(dt * read_int8("usrset", "DayDuty1")));   // unused
        write_int8("ledrbw", "duty2",
            static_cast<int8_t>(dt * 90));   // white
        write_int8("ledrbw", "duty3",
            static_cast<int8_t>(dt * read_int8("usrset", "DayDuty3")));   // red/blue
        disableRelaisLight();
    } else {
        write_int8("ledrbw", "duty0", read_int8("usrset", "DayDuty0"));   // blue
        write_int8("ledrbw", "duty1", read_int8("usrset", "DayDuty1"));   // ventilation
        write_int8("ledrbw", "duty2", read_int8("usrset", "DayDuty2"));   // white
        write_int8("ledrbw", "duty3", read_int8("usrset", "DayDuty3"));   // red/blue
        enableRelaisLight();
    }
}

void ManagementClass::enableHighPressurePump() {
    write_int8("relais0", "State", 1);
}

void ManagementClass::disableHighPressurePump() {
    write_int8("relais0", "State", 0);
}

void ManagementClass::enableRelaisLight() {
    write_int8("relais1", "State", 1);
}

void ManagementClass::disableRelaisLight() {
    write_int8("relais1", "State", 0);
}


void ManagementClass::enableDrainPump() {
    write_int8("hbrg0", "dc0_duty", 100);
}

void ManagementClass::disableDrainPump() {
    write_int8("hbrg0", "dc0_duty", 0);
}

void ManagementClass::enableOxyPump() {
    write_int8("hbrg2", "dc0_duty", 100);
}

void ManagementClass::disableOxyPump() {
    write_int8("hbrg2", "dc0_duty", 0);
}

void ManagementClass::write_obj_attribute(const char *objname,
                         const char *atrname,
                         char *buf,
                         int sz) {
    FwAttribute *atr = reinterpret_cast<FwAttribute *>(
                fw_get_object_attribute(objname, atrname));
    if (atr) {
        atr->write(buf, sz, false);
    } else {
        //uart_printf("%s:%s not found\r\n",
        //    objname, atrname);
    }
}

void ManagementClass::write_int8(const char *objname,
                                const char *atrname,
                                int8_t v) {
    char buf[2] = {static_cast<char>(v), 0};
    write_obj_attribute(objname, atrname, buf, 1);
}

void ManagementClass::write_uint32(const char *objname,
                                   const char *atrname,
                                   uint32_t v) {
    char buf[5] = {static_cast<char>(v),
                   static_cast<char>(v >> 8),
                   static_cast<char>(v >> 16),
                   static_cast<char>(v >> 24)};
    write_obj_attribute(objname, atrname, buf, 4);
}

int8_t ManagementClass::read_int8(const char *objname,
                                   const char *atrname) {
    FwAttribute *atr = reinterpret_cast<FwAttribute *>(
                fw_get_object_attribute(objname, atrname));
    if (atr) {
        return atr->to_int8();
    }
    return 0;
}

uint16_t ManagementClass::read_uint16(const char *objname,
                                      const char *atrname) {
    FwAttribute *atr = reinterpret_cast<FwAttribute *>(
                fw_get_object_attribute(objname, atrname));
    if (atr) {
        return atr->to_uint16();
    }
    return 0;
}

uint32_t ManagementClass::read_uint32(const char *objname,
                                      const char *atrname) {
    FwAttribute *atr = reinterpret_cast<FwAttribute *>(
                fw_get_object_attribute(objname, atrname));
    if (atr) {
        return atr->to_uint32();
    }
    return 0;
}

float ManagementClass::read_float32(const char *objname,
                                    const char *atrname) {
    FwAttribute *atr = reinterpret_cast<FwAttribute *>(
                fw_get_object_attribute(objname, atrname));
    if (atr) {
        return atr->to_float();
    }
    return 0;
}

uint32_t ManagementClass::getTimeOfDay() {
    uint32_t tow = 0;
    // [31:23] reserved
    // [22] PM
    // [21:20] Hours tens in BCD format
    // [19:16] Hours units in BCD format
    // [15] resrved
    // [14:12] Minutes tens in BCD format
    // [11:8] Minutes units in BCD format
    // [7] resered
    // [6:4] Seconds tens in BCD format
    // [3:0] Seconds units in BCD format
    uint32_t rtc_time = read_uint32("rtc", "Time");
    tow = ((rtc_time >> 20) & 0x3) * 10 * 3600; // seconds
    tow += ((rtc_time >> 16) & 0xF) * 3600;
    tow += ((rtc_time >> 12) & 0x7) * 10 * 60;
    tow += ((rtc_time >> 8) & 0xF) * 60;
    tow += ((rtc_time >> 4) & 0x7) * 10;
    tow += ((rtc_time >> 0) & 0xF);
    return tow;
}
