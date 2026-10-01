#ifndef TEMP_HUMIDITY_H
#define TEMP_HUMIDITY_H

typedef struct temp_humidity_reading {
    float humidity;
    float temp_celsius;
} temp_humidity_reading;

void InitTempHumidity();
void GetCurrentTempHumidity(temp_humidity_reading* result);


#endif