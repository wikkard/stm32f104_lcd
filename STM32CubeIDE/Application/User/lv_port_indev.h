#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#include "touch_calibration.h"

void lv_port_indev_init(void);
void lv_port_indev_process(void);
/* NULL disables pointer presses; every change requires a physical release. */
void lv_port_indev_set_calibration(const touch_calibration_t *calibration);

#endif
