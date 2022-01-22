/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306-spi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RX_BUF_LEN 255

#define DISP_MAX_X 18
#define DISP_MAX_Y 7
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
struct {
   uint16_t lastReceived;
   uint16_t lastProcessed;
} processor; 

struct {
   uint16_t x;
   uint16_t y;
} cursor; 

uint32_t dispTimer = 0; // Timer for DISPLAY task
uint32_t ledTimer = 0; // Timer for LED task

volatile uint8_t idlef = 0;
volatile uint8_t rx_buff[RX_BUF_LEN];

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void setup();
void runTaskManager();
void ledTask();
void displayTask();
void processBuffer();
void processCR();
void processLF();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int main(void) {
    setup();
    
    while(1) {
        runTaskManager();
    }
}

void setup() {
    HAL_Init();
    
    SystemClock_Config();
    
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    
    ssd1306_Init();

    ssd1306_Fill(Black);
    ssd1306_SetCursor(46, 28);
    ssd1306_WriteString("READY", Font_7x9, White);
    ssd1306_UpdateScreen();
    
    HAL_Delay(750);
    
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_UpdateScreen();
    
    __HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_IDLEF);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    
    HAL_UART_Receive_DMA(&huart1, rx_buff, RX_BUF_LEN);
    
}

void processCR() {
    cursor.x = 0;
    ssd1306_SetCursor(cursor.x*7, cursor.y*9);
}

void processLF() {
    if (cursor.y + 1 == DISP_MAX_Y) {
        //ssd1306_ShiftUp();
        ssd1306_Fill(Black);
        cursor.y = 0;
    } else {
        cursor.y++;
    }
    processCR();
}

void processBuffer() {
    idlef = 0;
    uint8_t x = 0;
    processor.lastProcessed = processor.lastReceived;
    processor.lastReceived = RX_BUF_LEN - hdma_usart1_rx.Instance->CNDTR;
    //ssd1306_SetCursor(0, 35);
    //ssd1306_WriteChar((processor.lastReceived - processor.lastProcessed) % 100 / 10 + 48, Font_7x9, White);
    //ssd1306_WriteChar((processor.lastReceived - processor.lastProcessed) % 10 + 48, Font_7x9, White);
    while (processor.lastProcessed != processor.lastReceived) {
        //x++;
        
        
        switch(rx_buff[processor.lastProcessed]) {

            case '\r':
                processCR();
                break; 
	        
            case '\n':
                processLF();
                break; 
          
            default : /* Optional */
                ssd1306_WriteChar(rx_buff[processor.lastProcessed], Font_7x9, White);
                if (cursor.x + 1 == DISP_MAX_X) {
                    processLF();
                } else {
                    cursor.x++;
                }
        }
        
        processor.lastProcessed = (processor.lastProcessed + 1) % RX_BUF_LEN;

    }
    //ssd1306_WriteChar((x) % 100 / 10 + 48, Font_7x9, White);
    //ssd1306_WriteChar((x) % 10 + 48, Font_7x9, White);
}

void runTaskManager() {
    if (ledTimer + 250 < HAL_GetTick()) {
        ledTask(); // runs each 250ms
        ledTimer = HAL_GetTick();
    }
    
    if (dispTimer + 100 < HAL_GetTick()) {
        displayTask(); // runs each 100ms
        dispTimer = HAL_GetTick();
    }
    
    if (idlef) {
        //ssd1306_SetCursor(0, 50);
        //ssd1306_WriteString("IDLE", Font_7x9, White);

        //HAL_UART_Transmit(&huart1, "OK\r\n", 4, 10);
        processBuffer();
    }
    
}

void ledTask() {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
}

void displayTask() {
    ssd1306_UpdateScreen();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
