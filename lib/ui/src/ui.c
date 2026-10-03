#include "UI.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void SetPixel(uint8_t *buf, int x,int y, bool on) {
    assert(x >= 0 && x < DISPLAY_WIDTH && y >=0 && y < DISPLAY_HEIGHT);

    // The calculation to determine the correct bit to set depends on which address
    // mode we are in. This code assumes horizontal

    // The video ram on the SSD1306 is split up in to 8 rows, one bit per pixel.
    // Each row is 128 long by 8 pixels high, each byte vertically arranged, so byte 0 is x=0, y=0->7,
    // byte 1 is x = 1, y=0->7 etc

    // This code could be optimised, but is like this for clarity. The compiler
    // should do a half decent job optimising it anyway.

    const int BytesPerRow = DISPLAY_WIDTH ; // x pixels, 1bpp, but each row is 8 pixel high, so (x / 8) * 8

    int byte_idx = (y / 8) * BytesPerRow + x;
    uint8_t byte = buf[byte_idx];

    if (on)
        byte |=  1 << (y % 8);
    else
        byte &= ~(1 << (y % 8));

    buf[byte_idx] = byte;
}

static void ClearSmallDigit(uint8_t *buf, Coord* coord) {
    int xStart = coord->x;
    int yStart = coord->y;
    
    const int FONT_WIDTH = 3;
    const int FONT_HEIGHT = 5;

    for (int x = 0; x < FONT_WIDTH; x++) {
        for (int k=0; k < FONT_HEIGHT; k++) {
            SetPixel(
                buf,
                xStart + x,
                yStart + k,
                false
            );
        }
    }
}

static void WriteSmallDigit(uint8_t *buf, int8_t number, Coord* coord) {
    int xStart = coord->x;
    int yStart = coord->y;
    
    const int FONT_WIDTH = 3;
    const int FONT_HEIGHT = 5;

    for (int x = 0; x < FONT_WIDTH; x++) {
        int idx = FONT_WIDTH * number + x;

        uint8_t data = small_numbers_spritesheet[idx];
        for (int k=0; k < FONT_HEIGHT; k++) {
            bool on = (data >> k) & 1;

            SetPixel(
                buf,
                xStart + x,
                yStart + k,
                on
            );
        }
    }
}

static void WriteTwoDigitsNumber(uint8_t *buf, int8_t number, Coord* coord) {
    if (number > 99) return;
    
    Coord localCoord;
    memcpy(&localCoord, coord, sizeof(Coord));

    if (number < 10) {
        WriteSmallDigit(buf, 0, &localCoord);

        localCoord.x += 4;
        WriteSmallDigit(buf, number, &localCoord);
        return;
    }

    int8_t n1 = number / 10;
    int8_t n2 = number % 10;

    WriteSmallDigit(buf, n1, &localCoord);

    localCoord.x += 4;
    WriteSmallDigit(buf, n2, &localCoord);
}

static void WriteThreeDigitsNumber(uint8_t *buf, int8_t number, Coord* coord) {
    if (number < 100 || number > 999) return;
    
    Coord localCoord;
    memcpy(&localCoord, coord, sizeof(Coord));

    int8_t n1 = number / 100;

    number = number % 100;
    int8_t n2 = number / 10;
    int8_t n3 = number % 10;

    WriteSmallDigit(buf, n1, &localCoord);

    localCoord.x += 4;
    WriteSmallDigit(buf, n2, &localCoord);

    localCoord.x += 4;
    WriteSmallDigit(buf, n3, &localCoord);
}

static void WriteFourDigitsNumber(uint8_t *buf, int16_t number, Coord* coord) {
    if (number > 9999) return;

    Coord localCoord;
    memcpy(&localCoord, coord, sizeof(Coord));

    WriteTwoDigitsNumber(buf, number / 100, &localCoord);
    localCoord.x += 8;

    WriteTwoDigitsNumber(buf, number % 100, &localCoord);
}

static void WriteTempNumber(
    uint8_t *buf,
    uint8_t n,
    Coord* coord
) {
    int16_t xStart = coord->x; 
    int16_t yStart = coord->y;

    const int FONT_WIDTH = 24;
    const int FONT_HEIGHT = 40;
    const int FONT_HEIGHT_BYTES = FONT_HEIGHT / 8;

    for (int page = 0; page < FONT_HEIGHT_BYTES; page++) {

        for (int x = 0; x < FONT_WIDTH; x++) {

            int idx =
                (FONT_WIDTH * FONT_HEIGHT_BYTES) * n +
                page * FONT_WIDTH +
                x;

            uint8_t data = temp_font_spritesheet[idx];

            for (int k = 0; k < 8; k++) {

                bool on = (data >> (7 - k)) & 1;

                SetPixel(
                    buf,
                    xStart + x,
                    yStart + page * 8 + k,
                    on
                );
            }
        }
    }
}



void WriteTemp(uint8_t *buf, uint8_t temp) {
    Coord StartFirst = {31, 20};
    Coord StartSecond = {58, 20};

    int firstNumber = temp / 10;
    int secondNumber = temp % 10;

    WriteTempNumber(buf, firstNumber, &StartFirst);
    WriteTempNumber(buf, secondNumber, &StartSecond);
}

static void WriteUmidity(uint8_t *buf, int8_t umidity) {
    Coord StartUmidity = {71, 6};

    if (umidity == 100) {
        WriteThreeDigitsNumber(buf, umidity, &StartUmidity);
        return;
    }

    WriteTwoDigitsNumber(buf, umidity, &StartUmidity);
    StartUmidity.x += 8;
    ClearSmallDigit(buf, &StartUmidity);
}


static void WriteIcon(uint8_t *buf, bool on, Coord* coord) {
    int xStart = coord->x;
    int yStart = coord->y;
    int idx;
    
    const int ICON_WIDTH = 4;
    const int ICON_HEIGHT = 3;

    for (int x = 0; x < ICON_WIDTH; x++) {
        if (!on) {
            idx = ICON_WIDTH + x;
        } else {
            idx = x;
        }

        uint8_t data = on_off_icons[idx];
        for (int k=0; k < ICON_HEIGHT; k++) {
            bool on = (data >> k) & 1;

            SetPixel(
                buf,
                xStart + x,
                yStart + k,
                on
            );
        }
    }
}

static void WriteDatetime(uint8_t *buf, int8_t hour, int8_t minute, int8_t day, int8_t month, int16_t year) {
    Coord StartHour = {49, 6};
    Coord StartMinute = {59, 6};
    Coord StartDay = {3, 6};
    Coord StartMonth = {17, 6};
    Coord StartYear = {31, 6};

    WriteTwoDigitsNumber(buf, hour, &StartHour);
    WriteTwoDigitsNumber(buf, minute, &StartMinute);
    WriteTwoDigitsNumber(buf, day, &StartDay);
    WriteTwoDigitsNumber(buf, month, &StartMonth);
    WriteFourDigitsNumber(buf, year, &StartYear);
}


static void WriteWifiStatus(uint8_t *buf, bool on) {
    Coord StartWifi = {99, 10};

    WriteIcon(buf, on, &StartWifi);
}

static void WriteSdStatus(uint8_t *buf, bool on) {
    Coord StartSd = {115, 10};

    WriteIcon(buf, on, &StartSd);
}

void InitUi(uint8_t *buf) {
    memcpy(buf, full_screen, 1024); // Maybe fix this later
}

void WriteUi(uint8_t *buf, GuiOptions* options) {
    WriteTemp(buf, options->temp);
    WriteUmidity(buf, options->umidity);
    WriteDatetime(buf, options->hour, options->minute, options->day, options->month, options->year);
    WriteWifiStatus(buf, options->wifi);
    WriteSdStatus(buf, options->sd);
}