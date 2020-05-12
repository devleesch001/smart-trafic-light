/*
 * inter_comuncation.h
 *
 *  Created on: May 12, 2020
 *      Author: dev-
 */

#ifndef INC_INTER_COM_H_
#define INC_INTER_COM_H_

/* Private includes ----------------------------------------------------------*/

// HAL
#include "stm32g0xx_hal.h"
#include "stm32g0xx_ll_system.h"

// OS
#include "cmsis_os2.h"

// Common
#include <stdio.h>		// sprintf & printf
#include <stdbool.h>	// bool
#include <string.h>		// strlen...

#define COM_HANDLER		huart1

#define TXT_IN_SIZE		255  /* max size buffer in */

#endif /* INC_INTER_COM_H_ */
