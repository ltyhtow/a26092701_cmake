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
  /* Minimal GPIO diagnostic: bypass generated init and FreeRTOS. */
  (void)HAL_Init();

  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
  (void)RCC->AHB2ENR;

  GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE10_Msk) |
                 GPIO_MODER_MODE10_0;
  GPIOB->OTYPER &= ~GPIO_OTYPER_OT10;
  GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED10_Msk;
  GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD10_Msk;
  GPIOB->BSRR = GPIO_BSRR_BS10;

  for (;;) {}
} /* end main */
