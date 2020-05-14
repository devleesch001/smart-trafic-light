/*
 * ToF.h
 *
 *  Created on: 14 mai 2020
 *      Author: Alexis DEVLEESCHAUWER
 */

#ifndef INC_TOF_H_
#define INC_TOF_H_

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

/* functions prototypes ---------------------------------------------*/

void VL53L_init(void);
void ToF_Read(void);


#endif /* INC_TOF_H_ */
