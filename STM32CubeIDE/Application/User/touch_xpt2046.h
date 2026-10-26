#ifndef TOUCH_XPT2046_H
#define TOUCH_XPT2046_H

#include <stdbool.h>
#include "main.h"

typedef struct
{
    bool pressed;
    bool valid;
    bool rail_value;
    uint16_t raw_x;
    uint16_t raw_y;
    uint32_t sample_count;
    uint32_t irq_count;
    uint32_t error_count;
    HAL_StatusTypeDef spi_status;
    uint32_t spi_error;
} touch_xpt2046_state_t;

void touch_xpt2046_init(SPI_HandleTypeDef *spi);
void touch_xpt2046_irq(void);
void touch_xpt2046_process(void);
const touch_xpt2046_state_t *touch_xpt2046_state(void);

#endif
