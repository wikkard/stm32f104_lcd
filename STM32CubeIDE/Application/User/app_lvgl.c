#include "app_config.h"

#if APP_USE_LVGL && !APP_TOUCH_TEST

#include "app_lvgl.h"
#include "firmware_version.h"
#include "lv_port_indev.h"
#include "touch_calibration_session.h"
#include "touch_calibration_store.h"
#include "lvgl/lvgl.h"

typedef enum
{
    VIEW_MAIN,
    VIEW_SETTINGS,
    VIEW_VERSION,
    VIEW_CALIBRATION,
    VIEW_SAVING,
    VIEW_SAVE_ERROR
} app_view_t;

typedef enum
{
    ACTION_NONE,
    ACTION_MAIN,
    ACTION_SETTINGS,
    ACTION_VERSION,
    ACTION_CALIBRATE,
    ACTION_CANCEL,
    ACTION_RETRY,
    ACTION_SESSION
} app_action_t;

static app_view_t view;
static app_action_t pending_action;
static lv_obj_t *counter_label;
static lv_timer_t *counter_timer;
static uint32_t counter;
static touch_calibration_t active;
static bool have_active;
static bool session_only;
static bool can_cancel;
static touch_calibration_session_t calibration_session;
static lv_obj_t *calibration_label;
static lv_obj_t *marker_horizontal;
static lv_obj_t *marker_vertical;
static uint32_t calibration_update;
static touch_store_result_t load_result;
static uint32_t saving_tick;

static lv_obj_t *make_label(int32_t x, int32_t y, int32_t width, const char *text)
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    LV_ASSERT_NULL(label);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, width);
    lv_label_set_text(label, text);
    return label;
}

static void request_action(lv_event_t *event)
{
    const app_action_t *action = lv_event_get_user_data(event);
    if(pending_action == ACTION_NONE) pending_action = *action;
}

static void make_button(int32_t x, int32_t y, int32_t width, const char *text,
                         const app_action_t *action)
{
    lv_obj_t *button = lv_button_create(lv_screen_active());
    LV_ASSERT_NULL(button);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, width, 38);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x245C80), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x387EA8), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_remove_flag(button, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(button, request_action, LV_EVENT_CLICKED, (void *)action);
    lv_obj_t *label = lv_label_create(button);
    LV_ASSERT_NULL(label);
    lv_label_set_text(label, text);
    lv_obj_center(label);
}

static const app_action_t action_main = ACTION_MAIN;
static const app_action_t action_settings = ACTION_SETTINGS;
static const app_action_t action_version = ACTION_VERSION;
static const app_action_t action_calibrate = ACTION_CALIBRATE;
static const app_action_t action_cancel = ACTION_CANCEL;
static const app_action_t action_retry = ACTION_RETRY;
static const app_action_t action_session = ACTION_SESSION;

static void begin_view(app_view_t next, const touch_calibration_t *mapping)
{
    /* Called only from the foreground, never inside a widget event callback. */
    lv_port_indev_set_calibration(NULL);
    if(counter_timer != NULL)
    {
        lv_timer_delete(counter_timer);
        counter_timer = NULL;
    }
    counter_label = NULL;
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(screen, lv_color_white(), 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    view = next;
    lv_port_indev_set_calibration(mapping);
}

static void update_counter(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    lv_label_set_text_fmt(counter_label, "Counter: %" LV_PRIu32, ++counter);
}

static void show_main(void)
{
    begin_view(VIEW_MAIN, &active);
    make_label(10, 12, 300, "STM32F103 + LVG TESTL");
    make_label(10, 34, 300, "TESTING 123");
    static const uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF};
    for(uint32_t i = 0; i < 3; ++i)
    {
        lv_obj_t *block = lv_obj_create(lv_screen_active());
        LV_ASSERT_NULL(block);
        lv_obj_remove_style_all(block);
        lv_obj_set_size(block, 72, 48);
        lv_obj_set_pos(block, 32 + (int32_t)i * 92, 66);
        lv_obj_set_style_bg_color(block, lv_color_hex(colors[i]), 0);
        lv_obj_set_style_bg_opa(block, LV_OPA_COVER, 0);
        lv_obj_remove_flag(block, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    }
    counter_label = make_label(10, 132, 300, "");
    lv_label_set_text_fmt(counter_label, "Counter: %" LV_PRIu32, counter);
    make_button(90, 166, 140, "Settings", &action_settings);
    make_label(10, 216, 300, session_only ? "Calibration: save unconfirmed" : "Calibration: saved");
    counter_timer = lv_timer_create(update_counter, 1000, NULL);
    LV_ASSERT_NULL(counter_timer);
}

static void show_settings(void)
{
    begin_view(VIEW_SETTINGS, &active);
    make_label(10, 12, 300, "Settings");
    make_button(40, 46, 240, "Recalibrate Touch", &action_calibrate);
    make_button(40, 96, 240, "Firmware Version", &action_version);
    make_button(40, 146, 240, "Back", &action_main);
    make_label(10, 210, 300, session_only ? "Calibration: save unconfirmed" : "Calibration: saved");
}

static void show_version(void)
{
    begin_view(VIEW_VERSION, &active);
    make_label(10, 24, 300, "Firmware Version");
    make_label(10, 74, 300, FIRMWARE_VERSION);
    make_label(10, 110, 300, "STM32F103ZGT6 / LVGL");
    make_button(90, 174, 140, "Back", &action_settings);
}

static void update_calibration_prompt(void)
{
    touch_point_t target = touch_calibration_target(&calibration_session);
    lv_obj_set_pos(marker_horizontal, target.x - 9, target.y - 1);
    lv_obj_set_pos(marker_vertical, target.x - 1, target.y - 9);
    lv_obj_set_y(calibration_label, calibration_session.target == 3 ? 146 : 68);
    const touch_xpt2046_state_t *touch = touch_xpt2046_state();
    const char *message = "Hold cross, then release.";
    if(touch->spi_status != HAL_OK) message = "SPI fault - retrying.";
    else if(touch->rail_value) message = "Rail value - check contact.";
    else if(calibration_session.progress == TOUCH_CAL_INVALID_TAPS) message = "Invalid taps - retry.";
    else if(calibration_session.progress == TOUCH_CAL_CENTER_MISMATCH) message = "Center mismatch - retry.";
    else if(calibration_session.waiting_release) message = "Release before tapping.";
    lv_label_set_text_fmt(calibration_label, "%s %u/4\n%s",
                          calibration_session.target == 3 ? "Verify center" : "Calibration",
                          (unsigned)calibration_session.target + 1, message);
}

static void start_calibration(void)
{
    can_cancel = have_active;
    touch_calibration_session_start(&calibration_session);
    begin_view(VIEW_CALIBRATION, can_cancel ? &active : NULL);
    make_label(10, 40, 300, can_cancel ? "Recalibrate touch" :
                                         touch_calibration_store_message(load_result));
    calibration_label = make_label(10, 68, 300, "");
    lv_obj_set_style_text_align(calibration_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_t **markers[] = {&marker_horizontal, &marker_vertical};
    for(uint32_t i = 0; i < 2; ++i)
    {
        *markers[i] = lv_obj_create(lv_screen_active());
        LV_ASSERT_NULL(*markers[i]);
        lv_obj_remove_style_all(*markers[i]);
        lv_obj_set_size(*markers[i], i == 0 ? 19 : 3, i == 0 ? 3 : 19);
        lv_obj_set_style_bg_color(*markers[i], lv_color_hex(0xFFFF00), 0);
        lv_obj_set_style_bg_opa(*markers[i], LV_OPA_COVER, 0);
        lv_obj_remove_flag(*markers[i], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    }
    /* Separate from all calibration targets, including the verification center. */
    if(can_cancel) make_button(110, 194, 100, "Cancel", &action_cancel);
    update_calibration_prompt();
    calibration_update = HAL_GetTick();
}

static void start_saving(void)
{
    begin_view(VIEW_SAVING, NULL);
    make_label(20, 80, 280, "Saving calibration...\nDo not remove power.");
    saving_tick = HAL_GetTick();
}

static void show_save_error(touch_store_result_t result)
{
    begin_view(VIEW_SAVE_ERROR, &calibration_session.transform);
    make_label(10, 12, 300, "Calibration save error");
    make_label(10, 42, 300, touch_calibration_store_message(result));
    make_label(10, 72, 300, "Previous saved data is retained.");
    make_button(40, 108, 240, "Retry", &action_retry);
    make_button(40, 158, 240, "Use for this session", &action_session);
}

static bool cancel_contact(const touch_xpt2046_state_t *touch)
{
    touch_point_t point;
    return can_cancel && touch->pressed && touch->valid &&
           touch_calibration_map(&active, (touch_point_t){touch->raw_x, touch->raw_y}, &point) &&
           point.x >= 110 && point.x < 210 && point.y >= 194 && point.y < 232;
}

void app_lvgl_init(void)
{
    lv_port_indev_init();
    touch_point_t raw[3];
    load_result = touch_calibration_store_load(raw, &active);
    have_active = load_result == TOUCH_STORE_OK;
    if(have_active)
    {
        show_main();
    }
    else
    {
        start_calibration();
    }
}

void app_lvgl_process(void)
{
    lv_port_indev_process();
    app_action_t action = pending_action;
    pending_action = ACTION_NONE;
    switch(action)
    {
        case ACTION_MAIN: show_main(); break;
        case ACTION_SETTINGS: show_settings(); break;
        case ACTION_VERSION: show_version(); break;
        case ACTION_CALIBRATE: start_calibration(); break;
        case ACTION_CANCEL:
            if(view == VIEW_CALIBRATION && can_cancel) show_settings();
            break;
        case ACTION_RETRY:
            if(view == VIEW_SAVE_ERROR) start_saving();
            break;
        case ACTION_SESSION:
            if(view == VIEW_SAVE_ERROR)
            {
                active = calibration_session.transform;
                have_active = true;
                session_only = true;
                show_main();
            }
            break;
        default: break;
    }

    if(view == VIEW_CALIBRATION)
    {
        const touch_xpt2046_state_t *touch = touch_xpt2046_state();
        if(cancel_contact(touch))
        {
            calibration_session.stable_count = 0;
            calibration_session.waiting_release = true;
        }
        else
        {
            touch_calibration_session_process(&calibration_session, touch);
        }
        if(calibration_session.progress == TOUCH_CAL_COMPLETE &&
           !calibration_session.waiting_release)
        {
            start_saving();
        }
        else if((uint32_t)(HAL_GetTick() - calibration_update) >= 100U)
        {
            calibration_update = HAL_GetTick();
            update_calibration_prompt();
        }
    }
    else if(view == VIEW_SAVING && (uint32_t)(HAL_GetTick() - saving_tick) >= 50U)
    {
        touch_store_result_t result = touch_calibration_store_save(calibration_session.raw);
        if(result == TOUCH_STORE_OK)
        {
            active = calibration_session.transform;
            have_active = true;
            session_only = false;
            show_main();
        }
        else
        {
            show_save_error(result);
        }
    }
}

#endif
