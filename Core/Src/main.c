/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_config.h"
#include "lcd.h"
#if APP_USE_LVGL
#include "app_lvgl.h"
#include "lv_port_disp.h"
#include "lvgl/lvgl.h"
#include "touch_xpt2046.h"
#if APP_TOUCH_TEST
#include "app_touch_test.h"
#endif
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define LCD_BASE 0x60000000UL

#define LCD_REG (*(volatile uint16_t *)0x60000000)
#define LCD_RAM (*(volatile uint16_t *)0x60000002)
//#define LCD_RAM (*(volatile uint16_t *)0x60000004)

#define BLACK   0x0000
#define WHITE   0xFFFF

#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F

#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define MAGENTA 0xF81F

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FSMC_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


static void LCD_WriteCommand(uint16_t cmd)
{
    LCD_REG = cmd;
}


static void LCD_WriteData(uint16_t data)
{
    LCD_RAM = data;
}


static void LCD_WriteReg(uint16_t reg, uint16_t value)
{
    LCD_WriteCommand(reg);
    LCD_WriteData(value);
}


#define LCD_RST_GPIO_Port GPIOA
#define LCD_RST_Pin       GPIO_PIN_7

#define LCD_BL_GPIO_PORT GPIOC
#define LCD_BL_Pin 6



void LCD_Reset(void)
{
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port,
                      LCD_RST_Pin,
                      GPIO_PIN_RESET);

    HAL_Delay(20);

    HAL_GPIO_WritePin(LCD_RST_GPIO_Port,
                      LCD_RST_Pin,
                      GPIO_PIN_SET);

    HAL_GPIO_WritePin(LCD_BL_GPIO_PORT,
    	LCD_BL_Pin,
                       GPIO_PIN_SET);

    HAL_Delay(120);
}




void LCD_Init(void)
{
    LCD_Reset();

    LCD_WriteCommand(0x3A);
    LCD_WriteData(0x55);

    LCD_WriteCommand(0xB5);
    LCD_WriteData(0x04);
    LCD_WriteData(0x04);
    LCD_WriteData(0x0A);
    LCD_WriteData(0x14);

    LCD_WriteCommand(0x35);
    LCD_WriteData(0x00);

    LCD_WriteCommand(0xCF);
    LCD_WriteData(0x00);
    LCD_WriteData(0xEA);
    LCD_WriteData(0xF0);

    LCD_WriteCommand(0xED);
    LCD_WriteData(0x64);
    LCD_WriteData(0x03);
    LCD_WriteData(0x12);
    LCD_WriteData(0x81);


    LCD_WriteCommand(0xE8);
    LCD_WriteData(0x85);
    LCD_WriteData(0x10);
    LCD_WriteData(0x78);

    LCD_WriteCommand(0xCB);
    LCD_WriteData(0x39);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x00);
    LCD_WriteData(0x33);
    LCD_WriteData(0x06);

    LCD_WriteCommand(0xF7);
    LCD_WriteData(0x20);

    LCD_WriteCommand(0xEA);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);

    LCD_WriteCommand(0xC0);
    LCD_WriteData(0x21);

    LCD_WriteCommand(0xC1);
    LCD_WriteData(0x10);

    LCD_WriteCommand(0xC5);
    LCD_WriteData(0x4F);
    LCD_WriteData(0x38);

    LCD_WriteCommand(0xC7);
    LCD_WriteData(0xB7);

    LCD_WriteCommand(0x36);
    LCD_WriteData(0x68);

    LCD_WriteCommand(0xB1);
    LCD_WriteData(0x00);
    LCD_WriteData(0x13);

    LCD_WriteCommand(0xB6);
    LCD_WriteData(0x0A);
    LCD_WriteData(0xA2);

    LCD_WriteCommand(0xF2);
    LCD_WriteData(0x02);

    LCD_WriteCommand(0x26);
    LCD_WriteData(0x01);

    LCD_WriteCommand(0xE0);
    LCD_WriteData(0x0F);
    LCD_WriteData(0x27);
    LCD_WriteData(0x24);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x10);
    LCD_WriteData(0x08);
    LCD_WriteData(0x55);
    LCD_WriteData(0x87);
    LCD_WriteData(0x45);
    LCD_WriteData(0x08);
    LCD_WriteData(0x14);
    LCD_WriteData(0x07);
    LCD_WriteData(0x13);
    LCD_WriteData(0x08);
    LCD_WriteData(0x00);

    LCD_WriteCommand(0xE1);
    LCD_WriteData(0x00);
    LCD_WriteData(0x0F);
    LCD_WriteData(0x12);
    LCD_WriteData(0x05);
    LCD_WriteData(0x11);
    LCD_WriteData(0x06);
    LCD_WriteData(0x25);
    LCD_WriteData(0x34);
    LCD_WriteData(0x37);
    LCD_WriteData(0x01);
    LCD_WriteData(0x08);
    LCD_WriteData(0x07);
    LCD_WriteData(0x2B);
    LCD_WriteData(0x34);
    LCD_WriteData(0x0F);

    LCD_WriteCommand(0x11);
    HAL_Delay(120);

    LCD_WriteCommand(0x29);
    HAL_Delay(20);
}


/*
void LCD_Init(void)
{
    LCD_Reset();

    LCD_WriteCommand(0x01);       // Software reset
    HAL_Delay(10);

    LCD_WriteCommand(0x28);       // Display OFF

    LCD_WriteCommand(0x3A);       // Pixel format
    LCD_WriteData(0x55);          // RGB565

    LCD_WriteCommand(0x36);       // Memory access control
    LCD_WriteData(0x48);

    LCD_WriteCommand(0x11);       // Sleep out
    HAL_Delay(120);

    LCD_WriteCommand(0x29);       // Display ON
    HAL_Delay(20);
}
*/


static void LCD_WriteData8(uint8_t data)
{
    LCD_RAM = data;
}


static void LCD_SetWindow(    uint16_t x0,    uint16_t y0,    uint16_t x1,    uint16_t y1)
{
    LCD_WriteCommand(0x2A);
    LCD_WriteData8(x0 >> 8);
    LCD_WriteData8(x0 & 0xFF);
    LCD_WriteData8(x1 >> 8);
    LCD_WriteData8(x1 & 0xFF);
    LCD_WriteCommand(0x2B);
    LCD_WriteData8(y0 >> 8);
    LCD_WriteData8(y0 & 0xFF);
    LCD_WriteData8(y1 >> 8);
    LCD_WriteData8(y1 & 0xFF);
    LCD_WriteCommand(0x2C);
}

void LCD_FillRect(uint16_t x,
                  uint16_t y,
                  uint16_t w,
                  uint16_t h,
                  uint16_t color)
{
    uint32_t i;

    LCD_SetWindow(x, y, x + w - 1, y + h - 1);

    for(i = 0; i < (uint32_t)w * h; i++)
    {
        LCD_WriteData(color);
    }
}


void LCD_FillScreen(uint16_t color)
{
    uint32_t i;

    LCD_SetWindow(0, 0, 319, 239);

    for(i = 0; i < 320UL * 240UL; i++)
    {
        LCD_WriteData(color);
    }

}


void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_SetWindow(x, y, x, y);
    LCD_WriteData(color);
}

void LCD_WritePixels(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                     const uint16_t *pixels)
{
    if(pixels == NULL || x0 > x1 || y0 > y1 ||
       x1 >= LCD_WIDTH || y1 >= LCD_HEIGHT)
    {
        Error_Handler();
        return;
    }

    uint32_t count = ((uint32_t)x1 - x0 + 1U) * ((uint32_t)y1 - y0 + 1U);
    LCD_SetWindow(x0, y0, x1, y1);
    for(uint32_t i = 0; i < count; ++i)
    {
        LCD_WriteData(pixels[i]);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
#if APP_USE_LVGL
  lv_port_disp_init();
  touch_xpt2046_init(&hspi2);
#if APP_TOUCH_TEST
  app_touch_test_init(&hspi2);
#else
  app_lvgl_init();
#endif
#endif
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */


    /* USER CODE BEGIN 3 */
#if APP_USE_LVGL
    touch_xpt2046_process();
#if APP_TOUCH_TEST
    app_touch_test_process();
#else
    app_lvgl_process();
#endif
    lv_timer_handler();
    HAL_Delay(5);
#else
    LCD_FillScreen(RED);
    HAL_Delay(1000);
    LCD_FillScreen(GREEN);
    HAL_Delay(1000);
    LCD_FillScreen(BLUE);
    HAL_Delay(1000);
    LCD_FillScreen(WHITE);
    HAL_Delay(1000);
#endif
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_4|LCD_BL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|LCD_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC4 LCD_BL_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4|LCD_BL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 LCD_RST_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|LCD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PG2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  HAL_GPIO_WritePin(TP_CS_GPIO_Port, TP_CS_Pin, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = TP_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TP_CS_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = TP_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(TP_IRQ_GPIO_Port, &GPIO_InitStruct);
  __HAL_GPIO_EXTI_CLEAR_IT(TP_IRQ_Pin);
  HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  /* USER CODE END MX_GPIO_Init_2 */
}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  /* Timing */
  Timing.AddressSetupTime = 2;
  Timing.AddressHoldTime = 1;
  Timing.DataSetupTime = 6;
  Timing.BusTurnAroundDuration = 2;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /** Disconnect NADV
  */

  __HAL_AFIO_FSMCNADV_DISCONNECTED();

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#if APP_USE_LVGL
  if(GPIO_Pin == TP_IRQ_Pin)
  {
    touch_xpt2046_irq();
  }
#else
  UNUSED(GPIO_Pin);
#endif
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
