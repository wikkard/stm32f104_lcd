#include "app_config.h"

#if APP_USE_LVGL && !APP_TOUCH_TEST

#include "lv_port_indev.h"
#include "touch_xpt2046.h"
#include "lvgl/lvgl.h"

static lv_indev_t *pointer;
static touch_calibration_t active_calibration;
static bool enabled;
static bool wait_release;
static uint32_t last_sample;
static uint32_t sample_tick;
static lv_point_t last_point;
static bool pointer_down;

void lv_port_indev_process(void)
{
    const touch_xpt2046_state_t *touch = touch_xpt2046_state();
    if(touch->sample_count != last_sample)
    {
        last_sample = touch->sample_count;
        sample_tick = HAL_GetTick();
    }
    if(!touch->pressed) wait_release = false;
    if(pointer_down && touch->pressed &&
       (!touch->valid || touch->rail_value || touch->spi_status != HAL_OK ||
        (uint32_t)(HAL_GetTick() - sample_tick) > 100U))
    {
        /* A broken sample stream must not turn a held press into a click. */
        lv_indev_reset(pointer, NULL);
        pointer_down = false;
        wait_release = true;
    }
}

static void read_pointer(lv_indev_t *indev, lv_indev_data_t *data)
{
    LV_UNUSED(indev);
    const touch_xpt2046_state_t *touch = touch_xpt2046_state();
    data->state = LV_INDEV_STATE_RELEASED;
    data->point = last_point;
    data->continue_reading = false;
    pointer_down = false;
    if(!enabled || wait_release || !touch->pressed || !touch->valid ||
       touch->rail_value || touch->spi_status != HAL_OK ||
       (uint32_t)(HAL_GetTick() - sample_tick) > 100U)
    {
        return;
    }
    touch_point_t point;
    if(!touch_calibration_map(&active_calibration,
                              (touch_point_t){touch->raw_x, touch->raw_y}, &point))
    {
        return;
    }
    if(point.x < 0) point.x = 0;
    if(point.x > 319) point.x = 319;
    if(point.y < 0) point.y = 0;
    if(point.y > 239) point.y = 239;
    last_point = (lv_point_t){point.x, point.y};
    data->point = last_point;
    data->state = LV_INDEV_STATE_PRESSED;
    pointer_down = true;
}

void lv_port_indev_set_calibration(const touch_calibration_t *calibration)
{
    enabled = calibration != NULL && calibration->divisor != 0;
    if(enabled) active_calibration = *calibration;
    wait_release = true;
    pointer_down = false;
    lv_indev_reset(pointer, NULL);
}

void lv_port_indev_init(void)
{
    pointer = lv_indev_create();
    LV_ASSERT_NULL(pointer);
    lv_indev_set_type(pointer, LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(pointer, lv_display_get_default());
    lv_indev_set_read_cb(pointer, read_pointer);
    lv_port_indev_set_calibration(NULL);
}

#endif
