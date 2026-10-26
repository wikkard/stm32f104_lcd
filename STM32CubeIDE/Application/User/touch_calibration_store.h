#ifndef TOUCH_CALIBRATION_STORE_H
#define TOUCH_CALIBRATION_STORE_H

#include "touch_calibration.h"

typedef enum
{
    TOUCH_STORE_OK,
    TOUCH_STORE_EMPTY,
    TOUCH_STORE_INVALID,
    TOUCH_STORE_LAYOUT,
    TOUCH_STORE_UNLOCK,
    TOUCH_STORE_ERASE,
    TOUCH_STORE_PROGRAM,
    TOUCH_STORE_VERIFY,
    TOUCH_STORE_LOCK
} touch_store_result_t;

touch_store_result_t touch_calibration_store_load(touch_point_t raw[3],
                                                  touch_calibration_t *calibration);
touch_store_result_t touch_calibration_store_save(const touch_point_t raw[3]);
const char *touch_calibration_store_message(touch_store_result_t result);

#endif
