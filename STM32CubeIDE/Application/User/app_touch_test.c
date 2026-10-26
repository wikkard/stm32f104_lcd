#include "app_config.h"

#if APP_TOUCH_TEST

#include "app_touch_test.h"
#include "touch_xpt2046.h"
#include "touch_calibration.h"
#include "touch_calibration_session.h"
#include "lvgl/lvgl.h"

static lv_obj_t *press_label;
static lv_obj_t *raw_label;
static lv_obj_t *counts_label;
static lv_obj_t *spi_label;
static lv_obj_t *hint_label;
static uint32_t last_update;
static lv_obj_t *calibration_label;
static lv_obj_t *marker_horizontal;
static lv_obj_t *marker_vertical;
static lv_obj_t *position_label;
static touch_calibration_session_t session;
static uint32_t last_sample;
static bool calibrated;

static void move_marker(touch_point_t point)
{
    lv_obj_set_pos(marker_horizontal, point.x - 9, point.y - 1);
    lv_obj_set_pos(marker_vertical, point.x - 1, point.y - 9);
}

static void show_diagnostics(bool show)
{
    lv_obj_t *labels[] = {press_label, raw_label, counts_label, spi_label,
                          hint_label, position_label};
    for(uint32_t i = 0; i < sizeof(labels) / sizeof(labels[0]); ++i)
    {
        if(show) lv_obj_remove_flag(labels[i], LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(labels[i], LV_OBJ_FLAG_HIDDEN);
    }
}

static void prompt_target(const char *prefix)
{
    move_marker(touch_calibration_target(&session));
    lv_obj_set_y(calibration_label, session.target == 3 ? 150 : 74);
    lv_label_set_text_fmt(calibration_label, "%s\n%s %u/4\nHold cross, then release.",
                          prefix, session.target == 3 ? "Verify center" : "Calibration",
                          (unsigned)session.target + 1);
}

static void process_calibration(const touch_xpt2046_state_t *touch)
{
    if(!calibrated)
    {
        touch_calibration_session_process(&session, touch);
        if(session.progress == TOUCH_CAL_COMPLETE && !session.waiting_release)
        {
            calibrated = true;
            lv_obj_add_flag(calibration_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(marker_horizontal, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(marker_vertical, LV_OBJ_FLAG_HIDDEN);
            show_diagnostics(true);
        }
        else if(session.progress == TOUCH_CAL_INVALID_TAPS)
        {
            prompt_target("Invalid taps - retry.");
        }
        else if(session.progress == TOUCH_CAL_CENTER_MISMATCH)
        {
            prompt_target("Center mismatch - retry.");
        }
        else
        {
            prompt_target(session.waiting_release ? "Release before next tap." :
                                                    "Hold steady on the cross.");
        }
        return;
    }
    if(!touch->pressed)
    {
        return;
    }
    if(!touch->valid || touch->rail_value)
    {
        return;
    }
    if(touch->sample_count == last_sample)
    {
        return;
    }
    last_sample = touch->sample_count;
    touch_point_t raw = {touch->raw_x, touch->raw_y};
    touch_point_t point;
    if(touch_calibration_map(&session.transform, raw, &point))
    {
        lv_label_set_text_fmt(position_label, "Screen X: %" LV_PRId32 "  Y: %" LV_PRId32,
                              point.x, point.y);
        if(point.x < 0) point.x = 0;
        if(point.x > 319) point.x = 319;
        if(point.y < 0) point.y = 0;
        if(point.y > 239) point.y = 239;
        move_marker(point);
        lv_obj_remove_flag(marker_horizontal, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(marker_vertical, LV_OBJ_FLAG_HIDDEN);
    }
}

static lv_obj_t *make_label(lv_obj_t *screen, int32_t y, const char *text)
{
    lv_obj_t *label = lv_label_create(screen);
    LV_ASSERT_NULL(label);
    lv_obj_set_width(label, 300);
    lv_obj_set_pos(label, 10, y);
    lv_label_set_text(label, text);
    return label;
}

void app_touch_test_init(SPI_HandleTypeDef *spi)
{
    LV_UNUSED(spi);
    touch_calibration_session_start(&session);
    lv_obj_t *screen = lv_screen_active();
    LV_ASSERT_NULL(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(screen, lv_color_white(), 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    press_label = make_label(screen, 64, "Waiting for touch...");
    raw_label = make_label(screen, 88, "X: ----  Y: ----  STALE");
    counts_label = make_label(screen, 112, "Samples: 0  IRQ: 0");
    spi_label = make_label(screen, 136, "SPI: starting");
    hint_label = make_label(screen, 176, "Press and move across the panel.");
    position_label = make_label(screen, 214, "Screen X: ----  Y: ----");
    calibration_label = make_label(screen, 74, "");
    lv_obj_set_style_text_align(calibration_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_t **markers[] = {&marker_horizontal, &marker_vertical};
    for(uint32_t i = 0; i < 2; ++i)
    {
        *markers[i] = lv_obj_create(screen);
        LV_ASSERT_NULL(*markers[i]);
        lv_obj_remove_style_all(*markers[i]);
        lv_obj_set_size(*markers[i], i == 0 ? 19 : 3, i == 0 ? 3 : 19);
        lv_obj_set_style_bg_color(*markers[i], lv_color_hex(0xFFFF00), 0);
        lv_obj_set_style_bg_opa(*markers[i], LV_OPA_COVER, 0);
    }
    show_diagnostics(false);
    prompt_target("Release, then tap targets.");
    last_update = HAL_GetTick() - 100U;
}

void app_touch_test_process(void)
{
    const touch_xpt2046_state_t *touch = touch_xpt2046_state();
    process_calibration(touch);
    if(!calibrated)
    {
        if(touch->spi_status != HAL_OK)
        {
            lv_label_set_text(calibration_label, "SPI fault - retrying.\nCheck touch wiring.");
        }
        else if(touch->pressed && touch->rail_value)
        {
            prompt_target("Rail value - check contact.");
        }
        return;
    }
    if(!touch->valid)
    {
        lv_obj_add_flag(marker_horizontal, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(marker_vertical, LV_OBJ_FLAG_HIDDEN);
    }
    uint32_t now = HAL_GetTick();
    if((uint32_t)(now - last_update) < 100U)
    {
        return;
    }
    last_update = now;
    lv_label_set_text_fmt(press_label, "PF1: %s / %s",
                          touch->pressed ? "LOW" : "HIGH",
                          touch->pressed ? "PRESSED" : "RELEASED");
    lv_label_set_text_fmt(raw_label, "X: %4u  Y: %4u  %s",
                          (unsigned)touch->raw_x, (unsigned)touch->raw_y,
                          touch->valid ? "VALID" : "STALE");
    lv_label_set_text_fmt(counts_label, "Samples: %" LV_PRIu32 "  IRQ: %" LV_PRIu32,
                          touch->sample_count, touch->irq_count);
    static const char *const statuses[] = {"OK", "ERROR", "BUSY", "TIMEOUT"};
    const char *status = touch->spi_status <= HAL_TIMEOUT ?
                         statuses[touch->spi_status] : "UNKNOWN";
    lv_label_set_text_fmt(spi_label, "SPI: %s  Failures: %" LV_PRIu32 "\nHAL: 0x%08" LV_PRIx32,
                          status, touch->error_count, touch->spi_error);
    lv_label_set_text(hint_label,
                      touch->spi_status != HAL_OK ? "SPI fault - retrying." :
                      touch->rail_value ? "Rail value: check wiring/contact." :
                      "Move to verify. Reset to calibrate.");
}

#endif
