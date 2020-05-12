/**
  ******************************************************************************
  * @file           : btn_driver.h
  * @brief          : Header for btn_driver.h.c file.
  *                   This file contains the common defines of the button module
  *                   for the user button on STM32 board.
  *
  *                   Use the shared btn_detect variable in your state machine
  *
  ******************************************************************************
**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BTN_DRIVER_H
#define __BTN_DRIVER_H

/* Private includes ----------------------------------------------------------*/
#include "inttypes.h"

/* Exported types ------------------------------------------------------------*/

typedef enum {
	BTN_NONE,
	BTN_SGL_CLICK,
} btn_detect_t;

/* Exported constants & variables --------------------------------------------*/

btn_detect_t btn_detect;

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initializes the button handler
  * @param  the global state pointer
  * @retval no value
 */
void btn_driver_init();

/* Private defines -----------------------------------------------------------*/

// single click detection
#define BTN_DEBOUNCE_MS		40

#endif
