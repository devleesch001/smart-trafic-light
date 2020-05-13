/*
 * ESP8266.h
 *
 *  Created on: May 12, 2020
 *      Author: Alexis DEVLEESCHAUWER
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

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

#define ESP_HANDLER		huart1
//#define ESP_HANDLER			hlpuart1

#define TIMEOUT_IN		10   /* 10 ms short time out for char input */
#define TIMEOUT_OUT		1000 /* 1 sec time out for text output */
#define TXT_IN_SIZE		255  /* max size buffer in */
#define TXT_TAG_SIZE	255  /* max size tag */
#define TXT_OUT_SIZE	255  /* max size buffer out */
#define INPUT_EOT		'\r' /* Carriage return */

/* Exported types ------------------------------------------------------------*/

typedef struct {
	uint32_t	id;
	char 		txt[TXT_OUT_SIZE];
} esp_obj_t;

osStatus_t esp8266(void);

char 		esp_msg[TXT_OUT_SIZE];
esp_obj_t 	esp_obj;

/* Exported macro ------------------------------------------------------------*/

osStatus_t 		espStatus;

extern osSemaphoreId_t 		esp_sHandle;
extern osMessageQueueId_t 	espQueueHandle;


/* Exported functions prototypes ---------------------------------------------*/

osStatus_t esp8266(void);
void esp8266_print(char * label, char * data);
uint8_t crc8(uint8_t *data);


#endif /* INC_ESP8266_H_ */
