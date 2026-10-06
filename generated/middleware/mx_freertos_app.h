/**
  ******************************************************************************
  * file           : mx_freertos_app.h
  * brief          : Header for mx_freertos_app.c
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
/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef MX_FREERTOS_APP_H
#define MX_FREERTOS_APP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <inttypes.h>       /* PRIx8 macros family */
#include <stdio.h>          /* printf */
#include "mx_hal_def.h"  /* aliases to the target-specific generated code */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "serial_protocol.h"
#include "imu_task.h"
#include "imu_fusion_task.h"
#include "imu_uart_test_task.h"
#include "motor_polarity_test_task.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
int32_t app_synctasks_init(void);

extern QueueHandle_t motion_command_queue;
extern QueueHandle_t pid_config_queue;
extern QueueHandle_t system_command_queue;
extern QueueHandle_t imu_sample_queue;
extern QueueHandle_t imu_fusion_input_queue;
extern QueueHandle_t imu_attitude_queue;

/** logging macro - just redirects to printf()
  * libc's stdout is redirected to UART thanks to the Basic stdio utility.
  *
  * @user You can enable logs by defining USE_TRACE=1 in the build options.
  */
#if defined(USE_TRACE) && USE_TRACE != 0
#define PRINTF(...)    printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif /* defined(USE_TRACE) && USE_TRACE != 0 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_FREERTOS_APP_H */
