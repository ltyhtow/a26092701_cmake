/**
  ******************************************************************************
  * @file           : mx_tim12.c
  * @brief          : Peripheral initialization
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
#include "mx_tim12.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions prototype------------------------------------------------*/
/* Exported variables by reference--------------------------------------------*/
static hal_tim_handle_t hTIM12;

/* Exported function definition ----------------------------------------------*/
/******************************************************************************/
/* Exported functions for TIM12 in HAL layer */
/******************************************************************************/
hal_tim_handle_t *mx_tim12_init(void)
{
  if (HAL_TIM_Init(&hTIM12, HAL_TIM12) != HAL_OK)
  {
    return NULL;
  }

  HAL_RCC_TIM12_EnableClock();

  /* Timer configuration to reach the output frequency at 2.197 kHz */
  hal_tim_config_t config;
  config.prescaler              = 0;
  config.counter_mode           = HAL_TIM_COUNTER_UP;
  config.period                 = 0xFFFF;
  config.repetition_counter     = 0;
  config.clock_sel.clock_source = HAL_TIM_CLK_INTERNAL;
  if (HAL_TIM_SetConfig(&hTIM12, &config) != HAL_OK)
  {
    return NULL;
  }

  /* Sampling Clock */
  if (HAL_TIM_SetDTSPrescaler(&hTIM12, HAL_TIM_DTS_DIV1) != HAL_OK)
  {
    return NULL;
  }
  if (HAL_TIM_SetDTS2Prescaler(&hTIM12, HAL_TIM_DTS2_DIV1) != HAL_OK)
  {
    return NULL;
  }

  hal_tim_oc_compare_unit_config_t oc_compare_unit_config;

  oc_compare_unit_config.mode  = HAL_TIM_OC_FROZEN;
  oc_compare_unit_config.pulse = 0x00000000;
  if (HAL_TIM_OC_SetConfigCompareUnit(&hTIM12, HAL_TIM_OC_COMPARE_UNIT_1,
                                      &oc_compare_unit_config) != HAL_OK)
  {
    return NULL;
  }

  oc_compare_unit_config.mode  = HAL_TIM_OC_FROZEN;
  oc_compare_unit_config.pulse = 0x00000000;
  if (HAL_TIM_OC_SetConfigCompareUnit(&hTIM12, HAL_TIM_OC_COMPARE_UNIT_2,
                                      &oc_compare_unit_config) != HAL_OK)
  {
    return NULL;
  }

  /* Update Event Management */
  if (HAL_TIM_SetUpdateSource(&hTIM12, HAL_TIM_UPDATE_REGULAR) != HAL_OK)
  {
    return NULL;
  }
  if (HAL_TIM_EnableUpdateGeneration(&hTIM12) != HAL_OK)
  {
    return NULL;
  }
  /* Master Mode Configuration */
  /* No GPIO configuration required for TIM12 */

  return &hTIM12;
}

void mx_tim12_deinit(void)
{
  (void)HAL_TIM_DeInit(&hTIM12);

  HAL_RCC_TIM12_DisableClock();

  HAL_RCC_TIM12_Reset();

  /* No GPIO de-initialization required for TIM12 */
}

hal_tim_handle_t *mx_tim12_gethandle(void)
{
  return &hTIM12;
}
