#ifndef TOUCH_CALIBRATION_H
#define TOUCH_CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int32_t x;
    int32_t y;
} touch_point_t;

typedef struct
{
    int64_t x[3];
    int64_t y[3];
    int64_t divisor;
} touch_calibration_t;

bool touch_calibration_fit(touch_calibration_t *calibration,
                           const touch_point_t raw[3], const touch_point_t screen[3]);
bool touch_calibration_map(const touch_calibration_t *calibration,
                           touch_point_t raw, touch_point_t *screen);

#endif
