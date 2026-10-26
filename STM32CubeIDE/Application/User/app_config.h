#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Set to 0 (or define APP_USE_LVGL=0 in the build) for the original LCD test. */
#ifndef APP_USE_LVGL
#define APP_USE_LVGL 1
#endif

#if APP_USE_LVGL != 0 && APP_USE_LVGL != 1
#error "APP_USE_LVGL must be 0 or 1"
#endif

/* Set to 1 for the standalone raw-touch/calibration diagnostic screen. */
#ifndef APP_TOUCH_TEST
#define APP_TOUCH_TEST 0
#endif

#if APP_TOUCH_TEST != 0 && APP_TOUCH_TEST != 1
#error "APP_TOUCH_TEST must be 0 or 1"
#endif

#if APP_TOUCH_TEST && !APP_USE_LVGL
#error "The touch diagnostic screen requires APP_USE_LVGL=1"
#endif

#endif
