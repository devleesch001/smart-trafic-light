/**
  ******************************************************************************
  * @file           : osdebug.c
  * @brief          : osdebug tools
  ******************************************************************************
**/

/* Includes ------------------------------------------------------------------*/
#include "osdebug.h"

/* Private includes ----------------------------------------------------------*/

#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

extern void Error_Handler(void);

/* Private variables ---------------------------------------------------------*/

extern UART_HandleTypeDef DEBUG_HANDLER; // serial monitor

#ifdef DEBUG

// global in module
dbg_out_t dbg_out;

// for message
char 			dbg_tag[TXT_TAG_SIZE];
char 			dbg_msg[TXT_OUT_SIZE];
logger_obj_t 	dbg_obj = {0, DBG_INFO, "NO MESSAGE"};
osStatus_t 		rstatus;


#endif

/* Private user code ---------------------------------------------------------*/

#ifdef DEBUG

void dbg_init(dbg_out_t out) {

	dbg_out = out;

	switch (dbg_out) {

	case DBG_UART:
		// nothing special
	break;


	default:
		Error_Handler();
	break;
	}

}

osStatus_t dbg(void) {

	//uint8_t			i;
	uint8_t			dbg_len;
	logger_obj_t	dbg_in;
	uint8_t			dbg_prio;
	uint32_t 		dbg_count;
	char			dbg_txt[TXT_OUT_SIZE];

	osStatus_t rstatus = osMessageQueueGet(log_queueHandle, &dbg_in, &dbg_prio, osWaitForever);

	if (rstatus == osOK) {

		switch (dbg_out) {

		case DBG_UART:
			dbg_count = osMessageQueueGetCount(log_queueHandle);
			snprintf(dbg_txt, TXT_OUT_SIZE, "\r\n{%lu|%lu} %s", dbg_count, dbg_in.id, dbg_in.txt);

			osSemaphoreAcquire(uart_sHandle, 0);

			// for (i=0; i<strlen(dbg_txt); i++)
			//	HAL_UART_Transmit(&DEBUG_HANDLER, (uint8_t *) dbg_txt+i, 1, TIMEOUT_OUT);

			 dbg_len = strlen(dbg_txt);
			 HAL_UART_Transmit(&DEBUG_HANDLER, (uint8_t *) dbg_txt, dbg_len, TIMEOUT_OUT);

			//printf("%s", dbg_txt);

			osSemaphoreRelease(uart_sHandle);

			break;

		default:
			rstatus = osErrorParameter;
			Error_Handler();
			break;
		}
	}

	return rstatus;
}

#else

osStatus_t dbg(dbg_out_t out) {
	return osOK;
}

#endif

// redefines output input standard io

// extern HAL_StatusTypeDef HAL_UART_Transmit(
// 	UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);

// extern HAL_StatusTypeDef HAL_UART_Receive(
// 	UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);

#ifdef __GNUC__

int __io_putchar(int ch)
{
   uint8_t ch8 = ch;
   HAL_UART_Transmit(&DEBUG_HANDLER, (uint8_t *)&ch8, 1, TIMEOUT_OUT);
   return ch;
}

int __io_getchar()
{
   uint8_t ch8;
   HAL_UART_Receive(&DEBUG_HANDLER, &ch8, 1, TIMEOUT_IN);
   return (int)ch8;
}

#else

int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&DEBUG_HANDLER, (uint8_t *)&ch, 1, TIMEOUT_OUT);
  return ch;
}

int __io_getchar()
{
   uint8_t ch8;
   HAL_UART_Receive(&DEBUG_HANDLER, &ch8, 1, TIMEOUT_IN);
   return (int)ch8;
}

#endif /* __GNUC__ */


