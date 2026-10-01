#include "TempHumidity.h"

#include <stdio.h>

static temp_humidity_reading readings[] = {
    {   
        .humidity = 38.00f,
        .temp_celsius = 35.6f
    },
    {   
        .humidity = 50.00f,
        .temp_celsius = 36.2f
    },
    {   
        .humidity = 12.00f,
        .temp_celsius = 38.6f
    },
    {   
        .humidity = 10.00f,
        .temp_celsius = 39.0f
    },
};

void InitTempHumidity() {
    printf("Initialized moock temp umidity\n");
}

void GetCurrentTempHumidity(temp_humidity_reading* result) {
    static int idx = 0;

    result->humidity = readings[idx % 4].humidity;
    result->temp_celsius = readings[idx % 4].temp_celsius;
    idx++;
}