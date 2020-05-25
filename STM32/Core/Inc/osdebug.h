/**
  ******************************************************************************
  * @file           : osdebug.h
  * @brief          : Header for osdebug.c file.
  *                   This file contains the debug tools.
  *
  *                   Define DEBUG or not to have trace or not
  *                   See DEBUG_UART : define LP UART 1 on PA2 tx & PA3 rx
  *
  *                   use debug macros for trace
  *                   dbg_printf with ticks
  *                   dbg_log with file + line
  *
  ******************************************************************************
**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OSDEBUG_H
#define __OSDEBUG_H

/* Private includes ----------------------------------------------------------*/

// HAL
#include "stm32g0xx_hal.h"
#include "stm32g0xx_ll_system.h"

// OS
#include "cmsis_os2.h"

// Common Libs
#include <stdio.h>		// sprintf & printf
#include <stdbool.h>	// bool
#include <string.h>		// strlen...


/* Exported constants --------------------------------------------------------*/

#define DEBUG_HANDLER		hlpuart1

#ifndef DEBUG /* to avoid warnings */
#define DEBUG /*to be defined or to be in comment*/
#endif /* to avoid warnings */

#define TIMEOUT_IN		10   /* 10 ms short time out for char input */
#define TIMEOUT_OUT		1000 /* 1 sec time out for text output */
#define TXT_IN_SIZE		255  /* max size buffer in */
#define TXT_TAG_SIZE	255  /* max size tag */
#define TXT_OUT_SIZE	255  /* max size buffer out */
#define INPUT_EOT		'\r' /* Carriage return */

/* Exported types ------------------------------------------------------------*/

typedef enum {
	DBG_INFO,
	DBG_WARNING,
	DBG_ERROR
} dbg_lvl_t;

// output
typedef enum {
	DBG_UART,
} dbg_out_t;

// object data type in queue
typedef struct {
	uint32_t	id;
	dbg_lvl_t	lvl;
	char 		txt[TXT_OUT_SIZE];
} logger_obj_t;

/* Exported macro ------------------------------------------------------------*/

#ifdef DEBUG /* DeBUGGING MODE */

// debug strings
char 			dbg_srnf[3];
char 			dbg_tag[TXT_TAG_SIZE];
char 			dbg_msg[TXT_OUT_SIZE];
char 			dbg_txt[TXT_OUT_SIZE];
logger_obj_t 	dbg_obj;
osStatus_t 		rstatus;

extern osSemaphoreId_t 		logger_sHandle;
extern osSemaphoreId_t 		uart_sHandle;
extern osMessageQueueId_t 	log_queueHandle;

#define TRACE true

/* macro to trace debug on serial monitor  with ticks */
#define dbg_printf(...) { \
	osSemaphoreAcquire(logger_sHandle, 0); \
		snprintf(dbg_msg, TXT_OUT_SIZE, __VA_ARGS__); /* args */ \
	osSemaphoreRelease(logger_sHandle); \
	rstatus = osMessageQueuePut(log_queueHandle, &dbg_msg, 0, 0); \
} /* end debug macro */

/* to have file name without path */
// #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define dbg_printfln(...) { \
	osSemaphoreAcquire(logger_sHandle, 0); \
		snprintf(dbg_txt, TXT_OUT_SIZE, __VA_ARGS__); \
		snprintf(dbg_msg, TXT_OUT_SIZE, "%s%s", dbg_txt, "\r\n"); \
	osSemaphoreRelease(logger_sHandle); \
	rstatus = osMessageQueuePut(log_queueHandle, &dbg_msg, 0, 0); \
}

/* macro to trace debug on serial monitor with file & line number */
#define dbg_log(...) { \
	osSemaphoreAcquire(logger_sHandle, 0); \
	snprintf(dbg_tag, TXT_OUT_SIZE, "%s [%d]", __FILENAME__, __LINE__);  /* file line tag */ \
	snprintf(dbg_msg, TXT_OUT_SIZE, __VA_ARGS__); /* args */ \
	snprintf(dbg_obj.txt, TXT_OUT_SIZE, "%s %s", dbg_tag, dbg_msg); /* concat */ \
	++dbg_obj.id; \
	osSemaphoreRelease(logger_sHandle); \
	rstatus = osMessageQueuePut(log_queueHandle, &dbg_obj, 0, 0); \
} /* end debug macro */

/* macro to trace debug on serial monitor with file & line number */

#else /* NO DEBUG */

#define TRACE false

/* macro are NOP */
#define dbg_printf(...) {}
#define dbg_log(...) {}

#endif /* DEBUG */

/* Exported functions prototypes ---------------------------------------------*/

// to be set in function logger_fct for thread loggerHandle

#ifdef DEBUG
void dbg_init(dbg_out_t out);
osStatus_t dbg();
#else
void dbg_init(dbg_out_t out);
osStatus_t dbg();
#endif /* DEBUG */

/* Private defines -----------------------------------------------------------*/

/* Redefines the weak functions :

#ifdef __GNUC__
int __io_putchar(int ch);
int __io_getchar();
#else
int fputc(int ch, FILE *f);
int __io_getchar();
#endif

*/

#endif /* __DEBUG_H */

