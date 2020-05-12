/******************************************************************************
 * @file           : btn_driver.c
 * @brief          : btn_driver.c file.
 *                   This file contains the common defines of the button module
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <osdebug.h>
#include "btn_driver.h"
#include "main.h"
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"


/* Private typedef -----------------------------------------------------------*/

typedef enum {
	BTN_NOT_INIT,
	BTN_IS_INIT
} btn_init_t;

typedef enum {
	BTN_TIMER_STOP,
	BTN_TIMER_START
} btn_timer_t;

typedef enum {
	BTN_RELEASED,
	BTN_PRESSED
} btn_state_t;

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

extern osMutexId_t freq_mutexHandle;

/**
 * btn states
 */

btn_init_t	is_init;
btn_state_t btn_state;
uint32_t 	last_pressed_tick;
uint32_t	last_unpressed_tick;

/**
 * @brief  initializes the button handler
 */
void btn_driver_init() {
	is_init 	= BTN_IS_INIT;
	btn_state 	= BTN_RELEASED;
	btn_detect	= BTN_NONE;
	last_pressed_tick 	= -1;
	last_unpressed_tick = -1;
}

/**
 * @brief  bounce while releasing
 * returns true if not a bounce
 */
bool btn_debounce(uint32_t tick)
{
	// time between the previous detection
	// avoid multiples detections
	return (HAL_GetTick() - tick > BTN_DEBOUNCE_MS) && (tick > 0) ? true : false;
}

/**
 * @brief  EXTI line detection callback. RELEASED
 * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
 * @retval None
 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	uint32_t now = HAL_GetTick();

	if ((GPIO_Pin == USER_BTN_Pin) && (is_init == BTN_IS_INIT)) {
		if (btn_debounce(last_unpressed_tick)) {
			last_unpressed_tick = now;
			btn_state = BTN_RELEASED;
			dbg_printf("btn released");
		}
		else dbg_printf("btn rising bounce");
	} else {
		dbg_printf("error rising btn driver");
		Error_Handler();
	}
}

/**
 * @brief  EXTI line detection callback. PRESSED
 * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
 * @retval None
 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	uint32_t now = HAL_GetTick();

	if ((GPIO_Pin == USER_BTN_Pin) && (is_init == BTN_IS_INIT) ) {
		if (btn_debounce(last_pressed_tick)) {
			last_pressed_tick = now;
			btn_state = BTN_PRESSED;
			dbg_printf("btn pressed");
		}
		else dbg_printf("btn falling bounce");
	} else {
		dbg_printf("error falling btn driver");
		Error_Handler();
	}
}
