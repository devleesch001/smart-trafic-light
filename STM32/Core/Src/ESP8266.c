/*
 * ESP8266.h
 *
 *  Created on: May 12, 2020
 *      Author: Alexis DEVLEESCHAUWER
 */

/* Includes ------------------------------------------------------------------*/
#include "ESP8266.h"
#include "osdebug.h"
/* Private includes ----------------------------------------------------------*/
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

extern void Error_Handler(void);

/* Private variables ---------------------------------------------------------*/

extern UART_HandleTypeDef ESP_HANDLER; // serial monitor
extern CRC_HandleTypeDef hcrc;

osStatus_t esp8266(void) {
	esp_obj_t		esp_in;
	uint8_t			esp_prio;

	osStatus_t rstatus = osMessageQueueGet(espQueueHandle, &esp_in, &esp_prio, osWaitForever);

	if (rstatus == osOK) {

		osSemaphoreAcquire(esp_sHandle, 0);

			HAL_UART_Transmit(&ESP_HANDLER, (uint8_t *)esp_in.txt, strlen(esp_in.txt), TIMEOUT_OUT);

		osSemaphoreRelease(esp_sHandle);
	}

	return rstatus;
}

void esp8266_print(char * phase, char * ToF) {

	osSemaphoreAcquire(esp_sHandle, 0);

		snprintf(esp_msg, TXT_OUT_SIZE, "{\"Status\":\"%s\",\"ToF\":\"%s\"}", phase, ToF);
		// char csum = crc8(esp_msg);
		uint32_t uwCRCValue = HAL_CRC_Calculate(&hcrc, (uint32_t *)esp_msg, strlen(esp_msg));
		snprintf(esp_obj.txt, TXT_OUT_SIZE, "\n{\"data\":%s,\"crc\":\"%lx\"}\r", esp_msg, uwCRCValue);
		//sniprintf(esp_obj.txt, TXT_OUT_SIZE, "%s", esp_msg);
		++esp_obj.id;

		espStatus = osMessageQueuePut(espQueueHandle, &esp_obj, 0, 0);

	osSemaphoreRelease(esp_sHandle);
}

char crc8(char *data)
{

    uint8_t crc = 0xff;
    size_t i, j;
    for (i = 0; i < strlen(data); i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }

    return crc;
}

