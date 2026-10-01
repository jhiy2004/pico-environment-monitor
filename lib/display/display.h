#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

typedef struct render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;

    int buflen;
} render_area;

void calc_render_area_buflen(render_area *area);

void displayInit();
void displayRender(uint8_t *buf, render_area *area);


#endif