/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - PIR + OLED + LED + Push Button
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "gpio.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdio.h>

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN 0 */
#define COUNTDOWN_TIME 10  // secondi di attesa
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();

  /* Inizializza OLED */
  ssd1306_Init();
  ssd1306_Fill(Black);
  ssd1306_SetCursor(10, 10);
  ssd1306_WriteString("Sistema PIR", Font_11x18, White);
  ssd1306_SetCursor(10, 40);
  ssd1306_WriteString("In attesa...", Font_7x10, White);
  ssd1306_UpdateScreen();

  /* LED iniziali */
  HAL_GPIO_WritePin(GPIOE, Blue_LED_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOE, Red_LED_Pin, GPIO_PIN_RESET);

  while (1)
  {
      GPIO_PinState pirState = HAL_GPIO_ReadPin(PIR_Signal_GPIO_Port, PIR_Signal_Pin);

      if (pirState == GPIO_PIN_SET)
      {
          // Movimento rilevato
          HAL_GPIO_WritePin(GPIOE, Red_LED_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOE, Blue_LED_Pin, GPIO_PIN_RESET);

          ssd1306_Fill(Black);
          ssd1306_SetCursor(10, 10);
          ssd1306_WriteString("MOVIMENTO!", Font_11x18, White);
          ssd1306_SetCursor(10, 35);
          ssd1306_WriteString("Attendere conferma", Font_7x10, White);
          ssd1306_UpdateScreen();

          uint8_t confirmed = 0;

          for (int t = COUNTDOWN_TIME; t >= 0; t--)
          {
              char buffer[32];
              ssd1306_Fill(Black);
              ssd1306_SetCursor(10, 10);
              ssd1306_WriteString("CONFERMA IN:", Font_7x10, White);

              sprintf(buffer, "%2d sec", t);
              ssd1306_SetCursor(40, 35);
              ssd1306_WriteString(buffer, Font_11x18, White);
              ssd1306_UpdateScreen();

              // Controlla se il pulsante è premuto
              if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == GPIO_PIN_RESET)
              {
                  confirmed = 1;
                  break;
              }

              HAL_Delay(1000);  // 1 secondo
          }

          ssd1306_Fill(Black);
          ssd1306_SetCursor(10, 20);

          if (confirmed)
          {
              ssd1306_WriteString("BIGLIETTO", Font_11x18, White);
              ssd1306_SetCursor(10, 45);
              ssd1306_WriteString("COMPRATO", Font_11x18, White);
              HAL_GPIO_WritePin(GPIOE, Red_LED_Pin, GPIO_PIN_SET);
              HAL_GPIO_WritePin(GPIOE, Blue_LED_Pin, GPIO_PIN_RESET);
          }
          else
          {
              ssd1306_WriteString("ANNULLAMENTO", Font_11x18, White);
              HAL_GPIO_WritePin(GPIOE, Red_LED_Pin, GPIO_PIN_RESET);
              HAL_GPIO_WritePin(GPIOE, Blue_LED_Pin, GPIO_PIN_SET);
          }

          ssd1306_UpdateScreen();
          HAL_Delay(3000);  // mostra il risultato per 3 s

          // Ripristina schermata base
          ssd1306_Fill(Black);
          ssd1306_SetCursor(10, 10);
          ssd1306_WriteString("Sistema PIR", Font_11x18, White);
          ssd1306_SetCursor(10, 40);
          ssd1306_WriteString("In attesa...", Font_7x10, White);
          ssd1306_UpdateScreen();

          HAL_GPIO_WritePin(GPIOE, Blue_LED_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOE, Red_LED_Pin, GPIO_PIN_RESET);
      }

      HAL_Delay(200);
  }
}

/**
  * @brief  Configurazione del clock
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|
                                RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    Error_Handler();

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    Error_Handler();
}

/**
  * @brief Gestione errori
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}
