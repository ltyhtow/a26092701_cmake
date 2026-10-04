/**
  ******************************************************************************
  * file           : main.c
  * brief          : Main program body
  *                  Calls target system initialization then loop in main.
  ******************************************************************************
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mx_freertos_app.h"
#include "mx_tim8.h"

volatile uint32_t g_diag_gpio_moder;
volatile uint32_t g_diag_gpio_otyper;
volatile uint32_t g_diag_gpio_odr;
volatile uint32_t g_diag_gpio_idr;

static void main_gpio_diagnostic(void)
{
  hal_gpio_config_t gpio_config;

  gpio_config.mode = HAL_GPIO_MODE_OUTPUT;
  gpio_config.speed = HAL_GPIO_SPEED_FREQ_LOW;
  gpio_config.pull = HAL_GPIO_PULL_NO;
  gpio_config.output_type = HAL_GPIO_OUTPUT_PUSHPULL;
  gpio_config.init_state = HAL_GPIO_PIN_RESET;

  (void)HAL_GPIO_Init(HAL_GPIOB,
                      MOTOR1_IN1_PIN | MOTOR1_IN2_PIN |
                      MOTOR2_IN1_PIN | MOTOR2_IN2_PIN,
                      &gpio_config);

  HAL_GPIO_WritePin(MOTOR1_IN1_PORT, MOTOR1_IN1_PIN, HAL_GPIO_PIN_SET);
  HAL_GPIO_WritePin(MOTOR1_IN2_PORT, MOTOR1_IN2_PIN, HAL_GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MOTOR2_IN1_PORT, MOTOR2_IN1_PIN, HAL_GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MOTOR2_IN2_PORT, MOTOR2_IN2_PIN, HAL_GPIO_PIN_RESET);

  /* Repeat the PB10 configuration at register level for debugger inspection. */
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
  GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE10_Msk) |
                 GPIO_MODER_MODE10_0;
  GPIOB->OTYPER &= ~GPIO_OTYPER_OT10;
  GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD10_Msk;
  GPIOB->BSRR = GPIO_BSRR_BS10;

  g_diag_gpio_moder = GPIOB->MODER;
  g_diag_gpio_otyper = GPIOB->OTYPER;
  g_diag_gpio_odr = GPIOB->ODR;
  g_diag_gpio_idr = GPIOB->IDR;
}

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions prototype -----------------------------------------------*/

/**
  * brief:  The application entry point.
  * retval: none but we specify int to comply with C99 standard
  */
int main(void)
{
  /** System Init: this code placed in targets folder initializes your system.
    * It calls the initialization (and sets the initial configuration) of the peripherals.
    * You can use STM32CubeMX to generate and call this code or not in this project.
    * It also contains the HAL initialization and the initial clock configuration.
    */
  if (mx_system_init() != SYSTEM_OK)
  {
    return (-1);
  }
  else
  {
    main_gpio_diagnostic();

    /*
      * You can start your application code here
      */

    app_synctasks_init();  //初始化freertos任务
    vTaskStartScheduler();

    while (1) {}
  }
} /* end main */
