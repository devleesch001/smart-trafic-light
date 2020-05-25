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
#include "main.h"
#include "cmsis_os.h"

// OS
#include "cmsis_os2.h"

// Common Libs
#include <stdio.h>		// sprintf & printf
#include <stdbool.h>	// bool
#include <string.h>		// strlen...


// API
#include "vl53l0x_api.h"

// Debug
#include "osdebug.h"
#include "ESP8266.h"

#define I2CHANDLE		hi2c1

/* functions prototypes ---------------------------------------------*/

void VL53L_init(void);
bool ToF_init(VL53L0X_DEV device, bool debug);
VL53L0X_Error getSingleRanging (VL53L0X_DEV device, uint16_t* Measure, bool debug);
void smartLight(uint16_t Measure);
void print_range_status(VL53L0X_RangingMeasurementData_t *pRangingMeasurementData);
void print_pal_error(VL53L0X_Error Status);

#endif /* INC_TOF_H_ */
