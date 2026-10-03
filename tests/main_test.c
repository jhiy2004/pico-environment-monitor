#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "display.h"
#include "UI.h"
#include "TempHumidity.h"
#include "logger.h"
#include "Wifi.h"

void testLogger() {
    log_info info = {
        .temperature = 20,
        .umidity = 87,
        .hour = 10,
        .minute = 30,
        .day = 27,
        .month = 9,
        .year = 2026
    };

    LogInit();
    LogInfo(&info);
}

void testUiWrites() {
    render_area area = {
        .start_col = 0,
        .end_col = 127,
        .start_page = 0,
        .end_page = 7
    };

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
    InitUi(buf);
    WriteUi(buf, &options);
    
    DisplayInit();
    DisplayRender(buf, &area);

    options.sd = true;
    options.wifi = true;
    options.temp = 0;
    options.umidity = 100;
    options.hour = 1;
    options.minute = 1;
    options.day = 1;
    options.month = 1;
    options.year = 1;

    WriteUi(buf, &options);
    DisplayRender(buf, &area);
}

void testTempHumidity() {
    InitTempHumidity();

    for (int i=0; i < 10; i++) {
        temp_humidity_reading reading = {0};
        GetCurrentTempHumidity(&reading);

        float fahrenheit = (reading.temp_celsius * 9 / 5) + 32;
        printf("Humidity = %.1f%%, Temperature = %.1fC (%.1fF)\n",
               reading.humidity, reading.temp_celsius, fahrenheit);
    }
}

void testWifi() {
    wifi_config config;
    GetWifiConfig(&config);

    temp_humidity_reading reading;


    InitWifi(&config);
    while(1) {
        GetCurrentTempHumidity(&reading);
        WebPoll(&reading);
    }
}

int main() {
    testUiWrites();
    testTempHumidity();
    testLogger();
    testWifi();

    return 0;
}