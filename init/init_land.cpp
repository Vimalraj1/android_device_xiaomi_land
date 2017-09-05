/*
   Copyright (c) 2017, The LineageOS Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/sysinfo.h>

#include <android-base/properties.h>
#include "property_service.h"
#include "vendor_init.h"

using android::base::GetProperty;
using android::base::SetProperty;

std::string heapstartsize;
std::string heapgrowthlimit;
std::string heapsize;
std::string heapminfree;
std::string heapmaxfree;
std::string large_cache_height;

static std::string board_id;

static void import_cmdline(const std::string& key,
        const std::string& value, bool for_emulator __attribute__((unused)))
{
    if (key.empty()) return;

    if (key == "board_id") {
        std::istringstream iss(value);
        std::string token;
        std::getline(iss, token, ':');
        board_id = token;
    }
}

static void init_alarm_boot_properties()
{
    int boot_reason;
    FILE *fp;

    fp = fopen("/proc/sys/kernel/boot_reason", "r");
    fscanf(fp, "%d", &boot_reason);
    fclose(fp);

    /*
     * Setup ro.alarm_boot value to true when it is RTC triggered boot up
     * For existing PMIC chips, the following mapping applies
     * for the value of boot_reason:
     *
     * 0 -> unknown
     * 1 -> hard reset
     * 2 -> sudden momentary power loss (SMPL)
     * 3 -> real time clock (RTC)
     * 4 -> DC charger inserted
     * 5 -> USB charger inserted
     * 6 -> PON1 pin toggled (for secondary PMICs)
     * 7 -> CBLPWR_N pin toggled (for external power supply)
     * 8 -> KPDPWR_N pin toggled (power key pressed)
     */
SetProperty("ro.alarm_boot", boot_reason == 3 ? "true" : "false");
}

void check_device()
{
    struct sysinfo sys;

    sysinfo(&sys);

    if (sys.totalram > 2048ull * 1024 * 1024) {
        // from - phone-xxhdpi-3072-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "288m";
        heapsize = "768m";
        heapminfree = "512k";
        heapmaxfree = "8m";
        large_cache_height = "1024";
    } else {
        // from - phone-xxhdpi-2048-dalvik-heap.mk
        heapstartsize = "16m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heapminfree = "2m";
        heapmaxfree = "8m";
        large_cache_height = "1024";
   }
}


void init_variant_properties()
{
    if (GetProperty("ro.cm.device") != "land")
        return;

    import_kernel_cmdline(0, import_cmdline);
    
    // Set board
    SetProperty("ro.product.wt.boardid", board_id.c_str());

    if (board_id == "S88537AA1") {
        SetProperty("ro.build.display.wtid", "SW_S88537AA1_V079_M20_MP_XM");
    } else if (board_id == "S88537AB1") {
        SetProperty("ro.build.display.wtid", "SW_S88537AB1_V079_M20_MP_XM");
    } else if (board_id == "S88537AC1") {
        SetProperty("ro.build.display.wtid", "SW_S88537AC1_V079_M20_MP_XM");
    } else if (board_id == "S88537BA1") {
        SetProperty("ro.build.display.wtid", "SW_S88537BA1_V079_M20_MP_XM");
        SetProperty("mm.enable.qcom_parser", "196495");
    } else if (board_id == "S88537CA1") {
        SetProperty("ro.build.display.wtid", "SW_S88537CA1_V079_M20_MP_XM");
        SetProperty("mm.enable.qcom_parser", "196495");
    } else if (board_id == "S88537EC1") {
        SetProperty("ro.build.display.wtid", "SW_S88537EC1_V079_M20_MP_XM");
        SetProperty("mm.enable.qcom_parser", "196495");
    }

    // Variants   
    if (board_id == "S88537AB1"){
        SetProperty("ro.product.model", "Redmi 3X");
    } else {
        SetProperty("ro.product.model", "Redmi 3S");
    }
}

void vendor_load_properties()
{
    init_alarm_boot_properties();
    check_device();
    init_variant_properties();

    SetProperty("dalvik.vm.heapstartsize", heapstartsize);
    SetProperty("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    SetProperty("dalvik.vm.heapsize", heapsize);
    SetProperty("dalvik.vm.heaptargetutilization", "0.75");
    SetProperty("dalvik.vm.heapminfree", heapminfree);
    SetProperty("dalvik.vm.heapmaxfree", heapmaxfree);

    SetProperty("ro.hwui.texture_cache_size", "72");
    SetProperty("ro.hwui.layer_cache_size", "48");
    SetProperty("ro.hwui.r_buffer_cache_size", "8");
    SetProperty("ro.hwui.path_cache_size", "32");
    SetProperty("ro.hwui.gradient_cache_size", "1");
    SetProperty("ro.hwui.drop_shadow_cache_size", "6");
    SetProperty("ro.hwui.texture_cache_flushrate", "0.4");
    SetProperty("ro.hwui.text_small_cache_width", "1024");
    SetProperty("ro.hwui.text_small_cache_height", "1024");
    SetProperty("ro.hwui.text_large_cache_width", "2048");
    SetProperty("ro.hwui.text_large_cache_height", large_cache_height);
}

