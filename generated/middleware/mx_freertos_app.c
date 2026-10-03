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
#define MOTOR_TEST_DUTY   720U
#define MOTOR_TEST_HOLD_MS 1000U
#define MOTOR_TEST_PAUSE_MS 500U
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*-------------------- Tasks definition --------------------------------------*/
/* Definitions for Task1 */
static TaskHandle_t Task1_Handle;

/* Private functions prototype -----------------------------------------------*/
/* Tasks entry function ------------------------------------------------------*/
static void function1(void *pvParameters);

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

static void motor_test_stop(hal_tim_handle_t *timer)
{
  motor_test_set_pulses(timer, 0U, 0U, 0U, 0U);
  (void)HAL_TIM_Stop(timer);
  (void)HAL_TIM_OC_StopChannel(timer, HAL_TIM_CHANNEL_1);
  (void)HAL_TIM_OC_StopChannel(timer, HAL_TIM_CHANNEL_2);
  (void)HAL_TIM_OC_StopChannel(timer, HAL_TIM_CHANNEL_3);
  (void)HAL_TIM_OC_StopChannel(timer, HAL_TIM_CHANNEL_4);
}

static void motor_test_hold(hal_tim_handle_t *timer,
                            uint32_t motor1_in1,
                            uint32_t motor1_in2,
                            uint32_t motor2_in1,
                            uint32_t motor2_in2,
                            TickType_t duration)
{
  motor_test_set_pulses(timer, motor1_in1, motor1_in2, motor2_in1, motor2_in2);
  vTaskDelay(duration);
  motor_test_set_pulses(timer, 0U, 0U, 0U, 0U);
  vTaskDelay(pdMS_TO_TICKS(MOTOR_TEST_PAUSE_MS));
}

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

  hal_tim_handle_t *tim8 = mx_tim8_gethandle();

  /* Start all four output channels once; a zero compare value keeps them low. */
  if ((HAL_TIM_OC_StartChannel(tim8, HAL_TIM_CHANNEL_1) != HAL_OK) ||
      (HAL_TIM_OC_StartChannel(tim8, HAL_TIM_CHANNEL_2) != HAL_OK) ||
      (HAL_TIM_OC_StartChannel(tim8, HAL_TIM_CHANNEL_3) != HAL_OK) ||
      (HAL_TIM_OC_StartChannel(tim8, HAL_TIM_CHANNEL_4) != HAL_OK) ||
      (HAL_TIM_Start(tim8) != HAL_OK))
  {
    for (;;) {
      HAL_GPIO_TogglePin(HAL_GPIOA, HAL_GPIO_PIN_3);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }

  /* AT8236: PWM on IN1/IN2 with the other input low gives fast-decay motion. */
  motor_test_hold(tim8, MOTOR_TEST_DUTY, 0U, 0U, 0U,
                  pdMS_TO_TICKS(MOTOR_TEST_HOLD_MS));
  motor_test_hold(tim8, 0U, MOTOR_TEST_DUTY, 0U, 0U,
                  pdMS_TO_TICKS(MOTOR_TEST_HOLD_MS));
  motor_test_hold(tim8, 0U, 0U, MOTOR_TEST_DUTY, 0U,
                  pdMS_TO_TICKS(MOTOR_TEST_HOLD_MS));
  motor_test_hold(tim8, 0U, 0U, 0U, MOTOR_TEST_DUTY,
                  pdMS_TO_TICKS(MOTOR_TEST_HOLD_MS));
  motor_test_hold(tim8, MOTOR_TEST_DUTY, 0U, MOTOR_TEST_DUTY, 0U,
                  pdMS_TO_TICKS(MOTOR_TEST_HOLD_MS));

  motor_test_stop(tim8);

  for(;;)
  {
    HAL_GPIO_TogglePin(HAL_GPIOA, HAL_GPIO_PIN_3);
    vTaskDelay(pdMS_TO_TICKS(500));

  }
}
