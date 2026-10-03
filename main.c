#include <pico/stdlib.h>
#include <pico/multicore.h>
#include "pico/util/datetime.h"
#include "hardware/rtc.h"



#include "display.h"
#include "ui.h"
#include "TempHumidity.h"
#include "logger.h"

#include "WifiConfig.h"
#include "Wifi.h"

#include "filesystem/vfs.h"

int main() {
    stdio_init_all();

    render_area area = {
        .start_col = 0,
        .end_col = 127,
        .start_page = 0,
        .end_page = 7
    };
    calc_render_area_buflen(&area);

    GuiOptions options = {
        .sd = false,
        .wifi = false,
        .temp = 38,
        .umidity = 8,
        .hour = 22,
        .minute = 49,
        .day = 24,
        .month = 9,
        .year = 2026
    };

    uint8_t buf[1024];

    log_info info = {
        .temperature = 20,
        .umidity = 87,
        .hour = 10,
        .minute = 30,
        .day = 27,
        .month = 9,
        .year = 2026
    };

    temp_humidity_reading reading = {};

    InitUi(buf);        
    displayInit();

    WriteUi(buf, &options);
    displayRender(buf, &area);


    if (!fs_init()) {
        printf("Failed to mount vfs filesystem");
    } else {
        options.sd = true;
    }

    WriteUi(buf, &options);
    displayRender(buf, &area);

    wifi_config config;
    GetWifiConfig(&config);

    if(InitWifi(&config)) {
        printf("Wifi Initialization completed\n");
        options.wifi = true;

        printf("IP: %s\n", config.ip);
    } else {
        printf("Wifi Initialization error\n");
    }

    InitTempHumidity();
    logInit();
    //logInfo(&info);

    // Delete the code below later
    //multicore_launch_core1(doSomething);
    
    char datetime_buf[256];
    char *datetime_str = &datetime_buf[0];

    // Start on Friday 5th of June 2020 15:45:00
    datetime_t t = {
            .year  = 2025,
            .month = 9,
            .day   = 5,
            .dotw  = 3, // 0 is Sunday, so 5 is Friday
            .hour  = 10,
            .min   = 16,
            .sec   = 0
    };

    // Start the RTC
    rtc_init();
    rtc_set_datetime(&t);

    // clk_sys is >2000x faster than clk_rtc, so datetime is not updated immediately when rtc_get_datetime() is called.
    // The delay is up to 3 RTC clock cycles (which is 64us with the default clock settings)
    sleep_us(64);

    uint64_t t1;
    while (1) {
        rtc_get_datetime(&t);

        options.day = t.day;
        options.month = t.month;
        options.year = t.year;
        options.hour = t.hour;
        options.minute = t.min;

        GetCurrentTempHumidity(&reading);
        t1 = time_us_64() + 3000000;

        options.temp = (int) reading.temp_celsius;
        options.umidity = (int) reading.humidity;

        WriteUi(buf, &options);
        displayRender(buf, &area);

        if (WebPoll(&reading)) {
            printf("Client hit");
        }
        
        sleep_until(t1);
    }
}