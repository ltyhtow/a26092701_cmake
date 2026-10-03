/**
  ******************************************************************************
  * @file           : mx_gpio_default.h
  * @brief          : Header for mx_gpio_default.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the mx_stm32c5xx_hal_drivers_license.md file
  * in the same directory as the generated code.
  * If no mx_stm32c5xx_hal_drivers_license.md file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_GPIO_DEFAULT_H
#define MX_GPIO_DEFAULT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32_hal.h"
#include "mx_def.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/******************************************************************************/
/* Exported defines for gpio_default in HAL layer                             */
/******************************************************************************/

/* Primary aliases for GPIO PA3 pin */
#define SYS_LED_PORT                                    HAL_GPIOA
#define SYS_LED_PIN                                     HAL_GPIO_PIN_3
#define SYS_LED_INIT_STATE                              HAL_GPIO_PIN_SET
#define SYS_LED_ACTIVE_STATE                            HAL_GPIO_PIN_SET
#define SYS_LED_INACTIVE_STATE                          HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PB1 pin */
#define MPU6050_INT_PORT                                HAL_GPIOB
#define MPU6050_INT_PIN                                 HAL_GPIO_PIN_1

/* Primary aliases for GPIO PB14 pin */
#define SYS_IO_PORT                                     HAL_GPIOB
#define SYS_IO_PIN                                      HAL_GPIO_PIN_14
#define SYS_IO_INIT_STATE                               HAL_GPIO_PIN_RESET
#define SYS_IO_ACTIVE_STATE                             HAL_GPIO_PIN_SET
#define SYS_IO_INACTIVE_STATE                           HAL_GPIO_PIN_RESET

/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/******************************************************************************/
/* Exported functions for gpio_default in HAL layer                           */
/******************************************************************************/
/**
  * @brief mx_gpio_default init function
  * This function configures the hardware resources used in this example
  * @retval 0  GPIO group correctly initialized
  * @retval -1 Issue detected during GPIO group initialization
  */
system_status_t mx_gpio_default_init(void);

/**
  * @brief  De-initialize gpio_default instance.
  */
system_status_t mx_gpio_default_deinit(void);

/**
  * @brief  Get the EXTI1 object.
  * @retval Pointer on the EXTI1 Handle
  */
hal_exti_handle_t *mx_gpio_default_exti1_gethandle(void);

/******************************************************************************/
/*                            EXTI Line1 interrupt                            */
/******************************************************************************/
void EXTI1_IRQHandler(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_GPIO_DEFAULT_H */
