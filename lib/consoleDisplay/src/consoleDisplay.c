#include "ConsoleDisplay.h"
#include "Display.h"

#include <stdio.h>
#include <stdbool.h>

static void printLine() {
    for (int i=0; i < 255; i++) {
        printf("=");
    }
    printf("\n");
}

void DisplayInit() {
    printf("Init console display");
}

void DisplayRender(uint8_t *buf, render_area *area) {
    printLine();

    for (int y = 0; y < BUF_HEIGHT * 8; y++) {
        for (int x = 0; x < BUF_WIDTH; x++) {

            int idx = (y / 8) * BUF_WIDTH + x;
            bool pixel = (buf[idx] >> (y % 8)) & 0x01;

            printf(pixel ? "██" : "  ");
        }

        printf("\n");
    }

    printLine();
}