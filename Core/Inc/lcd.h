#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_WIDTH 320U
#define LCD_HEIGHT 240U

/* Inclusive, in-bounds coordinates; pixels contains the complete RGB565 area. */
void LCD_WritePixels(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                     const uint16_t *pixels);

#endif
