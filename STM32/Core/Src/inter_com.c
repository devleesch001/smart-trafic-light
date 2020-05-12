/*
 * inter_com.c
 *
 *  Created on: May 12, 2020
 *      Author: Alexis DEVLEESCHAUWER
 */

#include "inter_com.h"

osStatus_t inter_Transmit(void){
	char data[255];
	char string1 = "test 1";
	char string2 = "test 2";

	snprintf(data, 255, "%s, %s", string1, string2);

	uint8_t datalen = strlen(data);
	//HAL_UART_Transmit(&INTER_HANDLER, (uint8_t *) data, datalen, 1000);

}

osStatus_t inter_Receive(void){

}

