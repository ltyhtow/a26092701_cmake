/**
  ******************************************************************************
  * @file           : mx_gpio_default.c
  * @brief          : gpio_default Peripheral initialization
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

/* Includes ------------------------------------------------------------------*/
#include "mx_gpio_default.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported variables by reference -------------------------------------------*/
static hal_exti_handle_t hEXTI1;

/******************************************************************************/
/* Exported functions for GPIO in HAL layer                                   */
/******************************************************************************/
system_status_t mx_gpio_default_init(void)
{
  hal_gpio_config_t  gpio_config;

  HAL_RCC_GPIOA_EnableClock();

  HAL_RCC_GPIOB_EnableClock();

  /*
    GPIO pin labels :
    PA3   ---------> SYS_LED
    */
  /* Configure PA3 GPIO pin in output mode */
  gpio_config.mode            = HAL_GPIO_MODE_OUTPUT;
  gpio_config.speed           = HAL_GPIO_SPEED_FREQ_LOW;
  gpio_config.pull            = HAL_GPIO_PULL_NO;
  gpio_config.output_type     = HAL_GPIO_OUTPUT_PUSHPULL;
  gpio_config.init_state      = SYS_LED_INIT_STATE;
  if (HAL_GPIO_Init(SYS_LED_PORT, SYS_LED_PIN, &gpio_config) != HAL_OK)
  {
    return SYSTEM_PERIPHERAL_ERROR;
  }

  /*
    GPIO pin labels :
    PB1   ---------> MPU6050_INT
    */
  /* Configure PB1 GPIO pin in input mode */
  gpio_config.mode            = HAL_GPIO_MODE_INPUT;
  gpio_config.pull            = HAL_GPIO_PULL_UP;
  if (HAL_GPIO_Init(MPU6050_INT_PORT, MPU6050_INT_PIN, &gpio_config) != HAL_OK)
  {
    return SYSTEM_PERIPHERAL_ERROR;
  }

  hal_exti_config_t exti_config;

  /* Initialize the EXTI for line 1 */
  HAL_EXTI_Init(&hEXTI1, HAL_EXTI_LINE_1);

  /* Set the trigger as RISING for the GPIOB */
  exti_config.trigger   = HAL_EXTI_TRIGGER_RISING;
  exti_config.gpio_port = HAL_EXTI_GPIOB;
  HAL_EXTI_SetConfig(&hEXTI1, &exti_config);

  /* Enable the INTERRUPT mode */
  HAL_EXTI_Enable(&hEXTI1, HAL_EXTI_MODE_INTERRUPT);

  /* Set line 1 Interrupt priority */
  HAL_CORTEX_NVIC_SetPriority(EXTI1_IRQn, HAL_CORTEX_NVIC_PREEMP_PRIORITY_5, HAL_CORTEX_NVIC_SUB_PRIORITY_0);
  HAL_CORTEX_NVIC_EnableIRQ(EXTI1_IRQn);

  /*
    GPIO pin labels :
    PB14  ---------> SYS_IO
    */
  /* Configure PB14 GPIO pin in output mode */
  gpio_config.mode            = HAL_GPIO_MODE_OUTPUT;
  gpio_config.speed           = HAL_GPIO_SPEED_FREQ_LOW;
  gpio_config.pull            = HAL_GPIO_PULL_NO;
  gpio_config.output_type     = HAL_GPIO_OUTPUT_PUSHPULL;
  gpio_config.init_state      = SYS_IO_INIT_STATE;
  if (HAL_GPIO_Init(SYS_IO_PORT, SYS_IO_PIN, &gpio_config) != HAL_OK)
  {
    return SYSTEM_PERIPHERAL_ERROR;
  }

  return SYSTEM_OK;
}

system_status_t mx_gpio_default_deinit(void)
{
  /* De-initialize the EXTI for GPIOB line1 */
  HAL_EXTI_DeInit(&hEXTI1);

  /* set line 1 Interrupt priority */
  HAL_CORTEX_NVIC_DisableIRQ(EXTI1_IRQn);

  /* De-initialize pins of GPIOA port */
  HAL_GPIO_DeInit(SYS_LED_PORT, SYS_LED_PIN);

  /* De-initialize pins of GPIOB port */
  HAL_GPIO_DeInit(HAL_GPIOB, MPU6050_INT_PIN | SYS_IO_PIN);

  return SYSTEM_OK;
}

hal_exti_handle_t *mx_gpio_default_exti1_gethandle(void)
{
  return &hEXTI1;
}

/******************************************************************************/
/*                            EXTI Line1 interrupt                            */
/******************************************************************************/
void EXTI1_IRQHandler(void)
{
  HAL_EXTI_IRQHandler(&hEXTI1);
}
