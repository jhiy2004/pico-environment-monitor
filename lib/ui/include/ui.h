#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stdbool.h>

#include "full_screen.h"
#include "temp_numbers.h"
#include "small_numbers.h"
#include "on_off_icons.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

typedef struct Coord {
    uint8_t x;
    uint8_t y;
} Coord;

typedef struct GuiOptions {
    bool sd;
    bool wifi;
    int8_t temp;
    int8_t umidity;
    int8_t hour;
    int8_t minute;
    int8_t day;
    int8_t month;
    int16_t year;
} GuiOptions;

void InitUi(uint8_t *buf);
void WriteUi(uint8_t *buf, GuiOptions* options);

#endif