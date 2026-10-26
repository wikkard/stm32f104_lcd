#ifndef TOUCH_CALIBRATION_SESSION_H
#define TOUCH_CALIBRATION_SESSION_H

#include "touch_calibration.h"
#include "touch_xpt2046.h"

typedef enum
{
    TOUCH_CAL_COLLECTING,
    TOUCH_CAL_INVALID_TAPS,
    TOUCH_CAL_CENTER_MISMATCH,
    TOUCH_CAL_COMPLETE
} touch_calibration_progress_t;

typedef struct
{
    touch_point_t raw[3];
    touch_calibration_t transform;
    touch_point_t anchor;
    touch_point_t sum;
    uint32_t target;
    uint32_t last_sample;
    uint32_t stable_count;
    bool waiting_release;
    touch_calibration_progress_t progress;
} touch_calibration_session_t;

void touch_calibration_session_start(touch_calibration_session_t *session);
void touch_calibration_session_process(touch_calibration_session_t *session,
                                       const touch_xpt2046_state_t *touch);
touch_point_t touch_calibration_target(const touch_calibration_session_t *session);
extern const touch_point_t touch_calibration_targets[3];

#endif
