#include "app_config.h"

#if APP_USE_LVGL

#include "touch_xpt2046.h"

#define TOUCH_SAMPLE_PERIOD_MS 20U
#define TOUCH_SPI_TIMEOUT_MS 5U
#define TOUCH_READ_X 0xD0U
#define TOUCH_READ_Y 0x90U

static SPI_HandleTypeDef *touch_spi;
static touch_xpt2046_state_t state;
static volatile uint32_t irq_count;
static uint32_t last_attempt;
static bool needs_rearm;
static bool needs_recovery;

static bool pen_down(void)
{
    return HAL_GPIO_ReadPin(TP_IRQ_GPIO_Port, TP_IRQ_Pin) == GPIO_PIN_RESET;
}

static HAL_StatusTypeDef read_axis(uint8_t command, uint16_t *value)
{
    uint8_t tx[3] = {command, 0, 0};
    uint8_t rx[3] = {0};
    HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(touch_spi, tx, rx,
                                                      sizeof(tx), TOUCH_SPI_TIMEOUT_MS);
    if(result == HAL_OK)
    {
        /* 24 clocks: command, one busy bit, 12 data bits, three trailing zeros. */
        *value = (uint16_t)((((uint16_t)rx[1] << 8) | rx[2]) >> 3) & 0x0FFFU;
    }
    return result;
}

static void record_error(HAL_StatusTypeDef result)
{
    state.spi_status = result;
    state.spi_error = HAL_SPI_GetError(touch_spi);
    state.error_count++;
    state.valid = false;
    state.rail_value = false;
    needs_rearm = true;
    needs_recovery = true;
}

void touch_xpt2046_init(SPI_HandleTypeDef *spi)
{
    if(spi == NULL || spi->Instance != SPI2)
    {
        Error_Handler();
        return;
    }
    touch_spi = spi;
    state = (touch_xpt2046_state_t){0};
    irq_count = 0;
    last_attempt = HAL_GetTick() - TOUCH_SAMPLE_PERIOD_MS;
    needs_rearm = true;
    needs_recovery = false;
    HAL_GPIO_WritePin(TP_CS_GPIO_Port, TP_CS_Pin, GPIO_PIN_SET);
}

void touch_xpt2046_irq(void)
{
    irq_count++;
}

void touch_xpt2046_process(void)
{
    state.irq_count = irq_count;
    state.pressed = pen_down();
    if(!state.pressed)
    {
        state.valid = false;
        state.rail_value = false;
    }

    uint32_t now = HAL_GetTick();
    if((uint32_t)(now - last_attempt) < TOUCH_SAMPLE_PERIOD_MS ||
       (!state.pressed && !needs_rearm))
    {
        return;
    }
    last_attempt = now;

    if(needs_recovery)
    {
        /* Reset the dedicated SPI bus after an interrupted/failed transfer. */
        HAL_StatusTypeDef result = HAL_SPI_DeInit(touch_spi);
        if(result == HAL_OK)
        {
            result = HAL_SPI_Init(touch_spi);
        }
        if(result != HAL_OK)
        {
            record_error(result);
            return;
        }
        needs_recovery = false;
    }

    uint16_t x = 0;
    uint16_t y = 0;
    bool rearming = needs_rearm;
    /* The datasheet recommends masking PENIRQ during conversions. */
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    HAL_GPIO_WritePin(TP_CS_GPIO_Port, TP_CS_Pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef result = HAL_OK;
    if(!rearming)
    {
        result = read_axis(TOUCH_READ_X, &x);
    }
    if(result == HAL_OK)
    {
        /* PD1:PD0=00 restores PENIRQ, including the startup/recovery path. */
        result = read_axis(TOUCH_READ_Y, &y);
    }
    HAL_GPIO_WritePin(TP_CS_GPIO_Port, TP_CS_Pin, GPIO_PIN_SET);

    /* Allow the panel/PENIRQ pull-up to settle before observing release. */
    HAL_Delay(1);
    __HAL_GPIO_EXTI_CLEAR_IT(TP_IRQ_Pin);
    HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    state.pressed = pen_down();

    if(result != HAL_OK)
    {
        record_error(result);
        return;
    }
    state.spi_status = HAL_OK;
    state.spi_error = HAL_SPI_ERROR_NONE;
    needs_rearm = false;
    if(!rearming && state.pressed)
    {
        state.raw_x = x;
        state.raw_y = y;
        state.valid = true;
        state.rail_value = x == 0 || x == 4095 || y == 0 || y == 4095;
        state.sample_count++;
    }
    else
    {
        state.valid = false;
        state.rail_value = false;
    }
}

const touch_xpt2046_state_t *touch_xpt2046_state(void)
{
    return &state;
}

#endif
