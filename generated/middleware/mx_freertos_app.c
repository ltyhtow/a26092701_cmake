/**
  ******************************************************************************
  * @file           : mx_freertos_app.c
  * @brief          : FreeRTOS initialization
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the mx_freertos_license.md file
  * in the same directory as the generated code.
  * If no mx_freertos_license.md file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "mx_freertos_app.h"
#include "FreeRTOS.h"
#include "projdefs.h"
#include "stm32c5xx_hal_gpio.h"
#include "mx_tim8.h"

/* Private define ------------------------------------------------------------*/
#define Task1_stack_size  128U
#define MOTOR_GPIO_DIAGNOSTIC 1U
#define MOTOR_TEST_DUTY   7199U
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*-------------------- Tasks definition --------------------------------------*/
/* Definitions for Task1 */
static TaskHandle_t Task1_Handle;

/* Private functions prototype -----------------------------------------------*/
/* Tasks entry function ------------------------------------------------------*/
static void function1(void *pvParameters);

#if !MOTOR_GPIO_DIAGNOSTIC
static void motor_test_set_pulses(hal_tim_handle_t *timer,
                                  uint32_t motor1_in1,
                                  uint32_t motor1_in2,
                                  uint32_t motor2_in1,
                                  uint32_t motor2_in2)
{
  (void)HAL_TIM_OC_SetCompareUnitPulse(timer, HAL_TIM_OC_COMPARE_UNIT_1, motor1_in1);
  (void)HAL_TIM_OC_SetCompareUnitPulse(timer, HAL_TIM_OC_COMPARE_UNIT_2, motor1_in2);
  (void)HAL_TIM_OC_SetCompareUnitPulse(timer, HAL_TIM_OC_COMPARE_UNIT_3, motor2_in1);
  (void)HAL_TIM_OC_SetCompareUnitPulse(timer, HAL_TIM_OC_COMPARE_UNIT_4, motor2_in2);
}
#endif

#if MOTOR_GPIO_DIAGNOSTIC
static void motor_gpio_set_diagnostic_levels(void)
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

  /* Motor 1 forward command: IN1 high, IN2 low. Motor 2 remains disabled. */
  HAL_GPIO_WritePin(MOTOR1_IN1_PORT, MOTOR1_IN1_PIN, HAL_GPIO_PIN_SET);
  HAL_GPIO_WritePin(MOTOR1_IN2_PORT, MOTOR1_IN2_PIN, HAL_GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MOTOR2_IN1_PORT, MOTOR2_IN1_PIN, HAL_GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MOTOR2_IN2_PORT, MOTOR2_IN2_PIN, HAL_GPIO_PIN_RESET);
}
#endif

/**
  * @brief Initializes FreeRTOS kernel objects.
  * @param None
  * @retval int32_t Returns 0 on success, -1 on failure.
  */
int32_t app_synctasks_init (void)
{
  BaseType_t ret;

  /* Task1 creation-------------------------------------*/
  ret = xTaskCreate(function1, "Task1", Task1_stack_size,
                    (void*) NULL, 0, &Task1_Handle);

  if (ret != pdPASS)
  {
      return -1;
  }

  return 0;
}

/* Tasks entry function ------------------------------------------------------*/
/**
  * @brief Function implementing the Task1 thread.
  * @param pvParameters: A pointer to the parameters passed to the task.
  * @retval None
  */
static void function1(void *pvParameters)
{
  ( void ) pvParameters;

#if MOTOR_GPIO_DIAGNOSTIC
  motor_gpio_set_diagnostic_levels();
  for (;;) {
    HAL_GPIO_TogglePin(HAL_GPIOA, HAL_GPIO_PIN_3);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
#else
  hal_tim_handle_t *tim8 = mx_tim8_gethandle();

  /* Load full-duty compare values before starting the timer. */
  motor_test_set_pulses(tim8, MOTOR_TEST_DUTY, 0U,
                        MOTOR_TEST_DUTY, 0U);
  (void)HAL_TIM_GenerateEvent(tim8, HAL_TIM_SW_EVENT_UPD);

  /* Start all four output channels once; the unused channels remain low. */
  if ((HAL_TIM_OC_StartChannel(tim8, HAL_TIM_CHANNEL_1) != HAL_OK) ||
      (HAL_TIM_OC_StartChannel(tim8, HAL_TIM_CHANNEL_2) != HAL_OK) ||
      (HAL_TIM_OC_StartChannel(tim8, HAL_TIM_CHANNEL_3) != HAL_OK) ||
      (HAL_TIM_OC_StartChannel(tim8, HAL_TIM_CHANNEL_4) != HAL_OK) ||
      (HAL_TIM_Start(tim8) != HAL_OK) ||
      (HAL_TIM_BREAK_EnableMainOutput(tim8) != HAL_OK))
  {
    for (;;) {
      HAL_GPIO_TogglePin(HAL_GPIOA, HAL_GPIO_PIN_3);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }

  for (;;) {
    HAL_GPIO_TogglePin(HAL_GPIOA, HAL_GPIO_PIN_3);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
#endif
}
