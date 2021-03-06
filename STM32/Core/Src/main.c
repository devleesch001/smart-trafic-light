/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "osdebug.h"
#include "ESP8266.h"
#include "vl53l0x_api.h"
#include "printf.h"
#include "ToF.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STX 0x02
#define ETX 0x03
#define LF  0x0a
#define CR  0x0d

#define VL53L_ADDR 0x29 << 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;

RTC_HandleTypeDef hrtc;

/* Definitions for task1 */
osThreadId_t task1Handle;
uint32_t task1_Buffer[ 512 ];
osStaticThreadDef_t task1_ControlBlock;
const osThreadAttr_t task1_attributes = {
  .name = "task1",
  .stack_mem = &task1_Buffer[0],
  .stack_size = sizeof(task1_Buffer),
  .cb_mem = &task1_ControlBlock,
  .cb_size = sizeof(task1_ControlBlock),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for task2 */
osThreadId_t task2Handle;
uint32_t task2_Buffer[ 128 ];
osStaticThreadDef_t task2_ControlBlock;
const osThreadAttr_t task2_attributes = {
  .name = "task2",
  .stack_mem = &task2_Buffer[0],
  .stack_size = sizeof(task2_Buffer),
  .cb_mem = &task2_ControlBlock,
  .cb_size = sizeof(task2_ControlBlock),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for logger */
osThreadId_t loggerHandle;
uint32_t logger_Buffer[ 128 ];
osStaticThreadDef_t logger_ControlBlock;
const osThreadAttr_t logger_attributes = {
  .name = "logger",
  .stack_mem = &logger_Buffer[0],
  .stack_size = sizeof(logger_Buffer),
  .cb_mem = &logger_ControlBlock,
  .cb_size = sizeof(logger_ControlBlock),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for espTask */
osThreadId_t espTaskHandle;
uint32_t espTaskBuffer[ 128 ];
osStaticThreadDef_t espTaskControlBlock;
const osThreadAttr_t espTask_attributes = {
  .name = "espTask",
  .stack_mem = &espTaskBuffer[0],
  .stack_size = sizeof(espTaskBuffer),
  .cb_mem = &espTaskControlBlock,
  .cb_size = sizeof(espTaskControlBlock),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for log_queue */
osMessageQueueId_t log_queueHandle;
uint8_t log_queueBuffer[ 32 * sizeof( logger_obj_t ) ];
osStaticMessageQDef_t log_queueControlBlock;
const osMessageQueueAttr_t log_queue_attributes = {
  .name = "log_queue",
  .cb_mem = &log_queueControlBlock,
  .cb_size = sizeof(log_queueControlBlock),
  .mq_mem = &log_queueBuffer,
  .mq_size = sizeof(log_queueBuffer)
};
/* Definitions for espQueue */
osMessageQueueId_t espQueueHandle;
uint8_t espQueueBuffer[ 8 * sizeof( esp_obj_t ) ];
osStaticMessageQDef_t espQueueControlBlock;
const osMessageQueueAttr_t espQueue_attributes = {
  .name = "espQueue",
  .cb_mem = &espQueueControlBlock,
  .cb_size = sizeof(espQueueControlBlock),
  .mq_mem = &espQueueBuffer,
  .mq_size = sizeof(espQueueBuffer)
};
/* Definitions for logger_s */
osSemaphoreId_t logger_sHandle;
osStaticSemaphoreDef_t logger_s_ControlBlock;
const osSemaphoreAttr_t logger_s_attributes = {
  .name = "logger_s",
  .cb_mem = &logger_s_ControlBlock,
  .cb_size = sizeof(logger_s_ControlBlock),
};
/* Definitions for uart_s */
osSemaphoreId_t uart_sHandle;
osStaticSemaphoreDef_t uart_s_ControlBlock;
const osSemaphoreAttr_t uart_s_attributes = {
  .name = "uart_s",
  .cb_mem = &uart_s_ControlBlock,
  .cb_size = sizeof(uart_s_ControlBlock),
};
/* Definitions for esp_s */
osSemaphoreId_t esp_sHandle;
osStaticSemaphoreDef_t esp_sControlBlock;
const osSemaphoreAttr_t esp_s_attributes = {
  .name = "esp_s",
  .cb_mem = &esp_sControlBlock,
  .cb_size = sizeof(esp_sControlBlock),
};
/* USER CODE BEGIN PV */

const char* hello	= "smart-trafic-light";

uint8_t btn_cpt = 0;

const osEventFlagsAttr_t flag_event_10s_attributes = {
  .name = "flag_event_10s",
  .attr_bits = 0,
  .cb_mem = NULL,
  .cb_size = 0
};

enum {
	GREEN,
	RED
};

uint16_t trigger;

bool flag = false;
bool buzzer = false;
uint8_t color = GREEN;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_CRC_Init(void);
void task1_fct(void *argument);
void task2_fct(void *argument);
void logger_fct(void *argument);
void espStartTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_LPUART1_UART_Init();
  MX_RTC_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

  dbg_init(DBG_UART);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of logger_s */
  logger_sHandle = osSemaphoreNew(1, 1, &logger_s_attributes);

  /* creation of uart_s */
  uart_sHandle = osSemaphoreNew(1, 1, &uart_s_attributes);

  /* creation of esp_s */
  esp_sHandle = osSemaphoreNew(1, 1, &esp_s_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of log_queue */
  log_queueHandle = osMessageQueueNew (32, sizeof(logger_obj_t), &log_queue_attributes);

  /* creation of espQueue */
  espQueueHandle = osMessageQueueNew (8, sizeof(esp_obj_t), &espQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of task1 */
  task1Handle = osThreadNew(task1_fct, NULL, &task1_attributes);

  /* creation of task2 */
  task2Handle = osThreadNew(task2_fct, NULL, &task2_attributes);

  /* creation of logger */
  loggerHandle = osThreadNew(logger_fct, NULL, &logger_attributes);

  /* creation of espTask */
  espTaskHandle = osThreadNew(espStartTask, NULL, &espTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  // first welcome hello
  dbg_log("%s TRACE DEBUG [%s]\r\n\r\n", hello, TRACE?"ON":"OFF" );

  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
 
  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage 
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_LPUART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00602173;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RED_LED_Pin|BUZZER_Pin|GREEN_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RED_LED_Pin BUZZER_Pin GREEN_LED_Pin */
  GPIO_InitStruct.Pin = RED_LED_Pin|BUZZER_Pin|GREEN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_BTN_Pin */
  GPIO_InitStruct.Pin = USER_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_LED_Pin */
  GPIO_InitStruct.Pin = USER_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USER_LED_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_task1_fct */
/**
  * @brief  Function implementing the task1 thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_task1_fct */
void task1_fct(void *argument)
{
  /* USER CODE BEGIN 5 */
	dbg_log("task 1 started");

	VL53L0X_Dev_t device;
	VL53L0X_Dev_t* pDevice = &device;

	uint16_t measure;
	uint16_t* pMeasure = &measure;


	bool ToFsensorAlready = ToF_init(pDevice, false);

	dbg_printfln("Triger set to %d", trigger);

	//dbg_printfln("ToF_init : %s", (ToF_init(pDevice, true)) ? "ok":"ko");


  /* Infinite loop */
	for(;;)
	{

		if (flag == true){
			if (color == GREEN){
				color = RED;
				HAL_GPIO_WritePin(GPIOC, RED_LED_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, GREEN_LED_Pin, GPIO_PIN_RESET);
				dbg_printfln("color is now set to RED");

			} else if (color == RED) {
				color = GREEN;
				HAL_GPIO_WritePin(GPIOC, GREEN_LED_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, RED_LED_Pin, GPIO_PIN_RESET);
				dbg_printfln("color is now set to GREEN");

			} else {
				color = GREEN;
				HAL_GPIO_WritePin(GPIOC, GREEN_LED_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, RED_LED_Pin, GPIO_PIN_RESET);
				dbg_printfln("color is default set to GREEN");
			}

			flag = false;
		}



		if (!ToFsensorAlready){
			dbg_printfln("ToF_init : ko");

			ToFsensorAlready = ToF_init(pDevice, false);

		} else {

			VL53L0X_Error Status = getSingleRanging(pDevice, pMeasure, false);

			if (Status == VL53L0X_ERROR_NONE){
				smartLight(measure);

			}

		}





		//dbg_printf("task 1 alive");

		osDelay(10);

	}
  /* USER CODE END 5 */ 
}

/* USER CODE BEGIN Header_task2_fct */
/**
* @brief Function implementing the task2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_task2_fct */
void task2_fct(void *argument)
{
  /* USER CODE BEGIN task2_fct */
	dbg_log("task 2 started\r\n");

	/* Infinite loop */
	for(;;)
	{
//		HAL_UART_Transmit(&huart1, "test1\r\n", 6, 1000);
//		HAL_UART_Transmit(&hlpuart1, "test1\r\n", 6, 1000);
		//dbg_printfln("task 2 alive");

		//esp8266_print("ToF", "triggered");

		if (buzzer){
			HAL_GPIO_WritePin(GPIOC, BUZZER_Pin, GPIO_PIN_SET);
			osDelay(2000);
			HAL_GPIO_WritePin(GPIOC, BUZZER_Pin, GPIO_PIN_RESET);
			buzzer = false;
		}

		osDelay(1000);
	}
  /* USER CODE END task2_fct */
}

/* USER CODE BEGIN Header_logger_fct */
/**
* @brief Function implementing the logger thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_logger_fct */
void logger_fct(void *argument)
{
  /* USER CODE BEGIN logger_fct */

	/* Infinite loop */
	for(;;)
	{
		dbg();
		osDelay(1);
	}
  /* USER CODE END logger_fct */
}

/* USER CODE BEGIN Header_espStartTask */
/**
* @brief Function implementing the espTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_espStartTask */
void espStartTask(void *argument)
{
  /* USER CODE BEGIN espStartTask */
  /* Infinite loop */
  for(;;)
  {
	  esp8266();
	  osDelay(1);
  }
  /* USER CODE END espStartTask */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin) {

	if (GPIO_Pin == USER_BTN_Pin) {
		flag = true;

	}
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
