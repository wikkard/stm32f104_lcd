#include "app_config.h"

#if APP_USE_LVGL

#include "touch_calibration.h"

bool touch_calibration_fit(touch_calibration_t *calibration,
                           const touch_point_t raw[3], const touch_point_t screen[3])
{
    *calibration = (touch_calibration_t){0};
    for(uint32_t i = 0; i < 3; ++i)
    {
        if(raw[i].x < 0 || raw[i].x > 4095 || raw[i].y < 0 || raw[i].y > 4095 ||
           screen[i].x < 0 || screen[i].x >= 320 || screen[i].y < 0 || screen[i].y >= 240)
        {
            return false;
        }
    }
    int64_t x1 = raw[1].x - raw[0].x;
    int64_t y1 = raw[1].y - raw[0].y;
    int64_t x2 = raw[2].x - raw[0].x;
    int64_t y2 = raw[2].y - raw[0].y;
    int64_t divisor = x1 * y2 - x2 * y1;
    /* Reject repeated/near-collinear taps and implausibly small raw spans. */
    if(divisor > -100000 && divisor < 100000)
    {
        return false;
    }
    int64_t *axes[2] = {calibration->x, calibration->y};
    for(uint32_t axis = 0; axis < 2; ++axis)
    {
        int64_t s0 = axis == 0 ? screen[0].x : screen[0].y;
        int64_t s1 = (axis == 0 ? screen[1].x : screen[1].y) - s0;
        int64_t s2 = (axis == 0 ? screen[2].x : screen[2].y) - s0;
        axes[axis][0] = s1 * y2 - s2 * y1;
        axes[axis][1] = x1 * s2 - x2 * s1;
        axes[axis][2] = s0 * divisor - axes[axis][0] * raw[0].x -
                         axes[axis][1] * raw[0].y;
    }
    calibration->divisor = divisor;
    return true;
}

bool touch_calibration_map(const touch_calibration_t *calibration,
                           touch_point_t raw, touch_point_t *screen)
{
    if(calibration->divisor == 0 || raw.x < 0 || raw.x > 4095 ||
       raw.y < 0 || raw.y > 4095)
    {
        return false;
    }
    screen->x = (int32_t)((calibration->x[0] * raw.x + calibration->x[1] * raw.y +
                           calibration->x[2]) / calibration->divisor);
    screen->y = (int32_t)((calibration->y[0] * raw.x + calibration->y[1] * raw.y +
                           calibration->y[2]) / calibration->divisor);
    return true;
}

#endif
