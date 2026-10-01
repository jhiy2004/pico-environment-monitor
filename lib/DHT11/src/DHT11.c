#include "TempHumidity.h"
#include "DHT11.h"
#include "config.h"

#include <stdio.h>
#include <pico/stdlib.h>

void InitTempHumidity() {
    gpio_init(DHT_PIN);
    gpio_set_dir(DHT_PIN, GPIO_OUT);

    printf("DHT11 Initialization completed\n");
}

void GetCurrentTempHumidity(temp_humidity_reading* result) {
    uint8_t data[5] = {0};

    // Inicia comunicação
    gpio_set_dir(DHT_PIN, GPIO_OUT);
    gpio_put(DHT_PIN, 0);
    sleep_ms(20);

    // Libera o barramento
    gpio_set_dir(DHT_PIN, GPIO_IN);
    gpio_pull_up(DHT_PIN);

    // Resposta do DHT11:
    // LOW ~80us
    while (gpio_get(DHT_PIN) == 1) {}
    while (gpio_get(DHT_PIN) == 0) {}
    while (gpio_get(DHT_PIN) == 1) {}

    // 40 bits
    for (int bit = 0; bit < 40; bit++) {

        // LOW ~50us
        while (gpio_get(DHT_PIN) == 0) {}

        // Mede HIGH
        uint32_t start = time_us_32();

        while (gpio_get(DHT_PIN) == 1) {}

        uint32_t duration = time_us_32() - start;

        data[bit / 8] <<= 1;

        if (duration > 40) {
            data[bit / 8] |= 1;
        }
    }

    printf(
        "Data: %02X %02X %02X %02X %02X\n",
        data[0],
        data[1],
        data[2],
        data[3],
        data[4]
    );

    uint8_t checksum =
        (data[0] + data[1] + data[2] + data[3]) & 0xFF;

    printf(
        "Checksum: %02X %02X\n",
        data[4],
        checksum
    );

    if (data[4] != checksum) {
        printf("Checksum INVALIDO!\n");
        return;
    }

    // DHT11:
    result->humidity = (float) ((data[0] << 8) + data[1]) / 10;
    if (result->humidity > 100) {
        result->humidity = data[0];
    }
    result->temp_celsius = (float) (((data[2] & 0x7F) << 8) + data[3]) / 10;
    if (result->temp_celsius > 125) {
        result->temp_celsius = data[2];
    }
    if (data[2] & 0x80) {
        result->temp_celsius = -result->temp_celsius;
    }
}