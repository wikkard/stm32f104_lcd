#include "app_config.h"

#if APP_USE_LVGL

#include "touch_calibration_session.h"

const touch_point_t touch_calibration_targets[3] = {{24, 24}, {295, 24}, {24, 215}};

void touch_calibration_session_start(touch_calibration_session_t *session)
{
    *session = (touch_calibration_session_t){.waiting_release = true};
}

touch_point_t touch_calibration_target(const touch_calibration_session_t *session)
{
    return session->target < 3 ? touch_calibration_targets[session->target] :
                                (touch_point_t){160, 120};
}

void touch_calibration_session_process(touch_calibration_session_t *session,
                                       const touch_xpt2046_state_t *touch)
{
    if(!touch->pressed)
    {
        session->waiting_release = false;
        session->stable_count = 0;
        return;
    }
    if(!touch->valid || touch->rail_value || touch->spi_status != HAL_OK)
    {
        session->stable_count = 0;
        return;
    }
    if(session->waiting_release || session->progress == TOUCH_CAL_COMPLETE ||
       session->last_sample == touch->sample_count)
    {
        return;
    }
    session->last_sample = touch->sample_count;
    touch_point_t raw = {touch->raw_x, touch->raw_y};
    if(session->stable_count == 0 ||
       raw.x - session->anchor.x > 50 || session->anchor.x - raw.x > 50 ||
       raw.y - session->anchor.y > 50 || session->anchor.y - raw.y > 50)
    {
        session->anchor = raw;
        session->sum = (touch_point_t){0};
        session->stable_count = 0;
    }
    session->sum.x += raw.x;
    session->sum.y += raw.y;
    if(++session->stable_count < 8)
    {
        return;
    }
    raw = (touch_point_t){session->sum.x / 8, session->sum.y / 8};
    session->stable_count = 0;
    session->waiting_release = true;
    session->progress = TOUCH_CAL_COLLECTING;
    if(session->target < 3)
    {
        session->raw[session->target++] = raw;
        if(session->target == 3 &&
           !touch_calibration_fit(&session->transform, session->raw, touch_calibration_targets))
        {
            session->target = 0;
            session->progress = TOUCH_CAL_INVALID_TAPS;
        }
    }
    else
    {
        touch_point_t point;
        if(!touch_calibration_map(&session->transform, raw, &point) ||
           point.x < 145 || point.x > 175 || point.y < 105 || point.y > 135)
        {
            session->target = 0;
            session->transform = (touch_calibration_t){0};
            session->progress = TOUCH_CAL_CENTER_MISMATCH;
        }
        else
        {
            session->progress = TOUCH_CAL_COMPLETE;
        }
    }
}

#endif
