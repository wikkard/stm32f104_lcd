#include "app_config.h"

#if APP_USE_LVGL

#include "lv_port_disp.h"
#include "lcd.h"
#include "main.h"
#include "lvgl/lvgl.h"

#define LCD_BUFFER_LINES 24U

static uint16_t draw_buffer[LCD_WIDTH * LCD_BUFFER_LINES]
    __attribute__((aligned(LV_DRAW_BUF_ALIGN)));

static void display_flush(lv_display_t *display, const lv_area_t *area,
                          uint8_t *px_map)
{
    LV_ASSERT(area->x1 >= 0 && area->y1 >= 0);
    LV_ASSERT(area->x2 >= area->x1 && area->y2 >= area->y1);
    LV_ASSERT(area->x2 < (int32_t)LCD_WIDTH && area->y2 < (int32_t)LCD_HEIGHT);

    /* Stride alignment is 1 byte, so RGB565 rows are tightly packed. */
    LCD_WritePixels((uint16_t)area->x1, (uint16_t)area->y1,
                    (uint16_t)area->x2, (uint16_t)area->y2,
                    (const uint16_t *)px_map);
    lv_display_flush_ready(display);
}

void lv_port_disp_init(void)
{
    lv_init();
    lv_tick_set_cb(HAL_GetTick);

    lv_display_t *display = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    LV_ASSERT_NULL(display);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, draw_buffer, NULL, sizeof(draw_buffer),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, display_flush);
}

#endif
