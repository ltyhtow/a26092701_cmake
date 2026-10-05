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
#include "lwpkt/lwpkt.h"
#include "semphr.h"
#include "serial_transport.h"
#include "stm32c5xx_hal_gpio.h"

#include <string.h>

/* Private define ------------------------------------------------------------*/
#define Task1_stack_size  128U
#define ProtocolTask_stack_size  384U
#define PROTOCOL_TASK_PERIOD_MS 5U
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*-------------------- Tasks definition --------------------------------------*/
/* Definitions for Task1 */
static TaskHandle_t Task1_Handle;
static TaskHandle_t ProtocolTask_Handle;
static lwpkt_t protocol_packet;
static SemaphoreHandle_t protocol_tx_mutex;
static volatile uint8_t protocol_ready;

QueueHandle_t motion_command_queue;
QueueHandle_t pid_config_queue;
QueueHandle_t system_command_queue;

/* Private functions prototype -----------------------------------------------*/
/* Tasks entry function ------------------------------------------------------*/
static void function1(void *pvParameters);
static void protocol_task(void *pvParameters);
static void protocol_dispatch_packet(const lwpkt_t *packet);

/**
  * @brief Initializes FreeRTOS kernel objects.
  * @param None
  * @retval int32_t Returns 0 on success, -1 on failure.
  */
int32_t app_synctasks_init (void)
{
  BaseType_t ret;

  motion_command_queue = xQueueCreate(1U, sizeof(motion_command_t));
  pid_config_queue = xQueueCreate(2U, sizeof(pid_config_command_t));
  system_command_queue = xQueueCreate(4U, sizeof(system_command_t));
  protocol_tx_mutex = xSemaphoreCreateMutex();

  if (motion_command_queue == NULL || pid_config_queue == NULL || system_command_queue == NULL ||
      protocol_tx_mutex == NULL)
  {
      return -1;
  }

  ret = xTaskCreate(protocol_task, "Protocol", ProtocolTask_stack_size,
                    (void*) NULL, 2U, &ProtocolTask_Handle);

  if (ret != pdPASS || serial_transport_init(ProtocolTask_Handle) != pdPASS)
  {
      return -1;
  }

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

  for(;;)
  {
    HAL_GPIO_TogglePin(HAL_GPIOA, HAL_GPIO_PIN_3);
    vTaskDelay(pdMS_TO_TICKS(500));

  }
}

static void protocol_dispatch_packet(const lwpkt_t *packet)
{
  const uint8_t *data = (const uint8_t *)lwpkt_get_data(packet);
  const size_t length = lwpkt_get_data_len(packet);
  const uint32_t command = lwpkt_get_cmd(packet);

  if (data == NULL)
  {
      return;
  }

  switch (command)
  {
    case SERIAL_CMD_MOTION:
    {
      motion_command_t value;
      if (length == sizeof(value))
      {
          memcpy(&value, data, sizeof(value));
          if (value.version == SERIAL_PROTOCOL_VERSION && value.reserved[0] == 0U &&
              value.reserved[1] == 0U && (value.flags & ~0x0003U) == 0U)
          {
              (void)xQueueOverwrite(motion_command_queue, &value);
          }
      }
      break;
    }

    case SERIAL_CMD_PID_CONFIG:
    {
      pid_config_command_t value;
      if (length == sizeof(value))
      {
          memcpy(&value, data, sizeof(value));
          if (value.version == SERIAL_PROTOCOL_VERSION && value.reserved == 0U &&
              value.loop_id <= 2U)
          {
              (void)xQueueSend(pid_config_queue, &value, 0U);
          }
      }
      break;
    }

    case SERIAL_CMD_SYSTEM:
    {
      system_command_t value;
      if (length == sizeof(value))
      {
          memcpy(&value, data, sizeof(value));
          if (value.version == SERIAL_PROTOCOL_VERSION && value.reserved == 0U &&
              value.action >= 1U && value.action <= 4U)
          {
              (void)xQueueSend(system_command_queue, &value, 0U);
          }
      }
      break;
    }

    default:
      /* Unknown commands are ignored until an application response is added. */
      break;
  }
}

static void protocol_task(void *pvParameters)
{
  lwrb_t *rx_buffer = serial_transport_rx_buffer();
  lwrb_t *tx_buffer = serial_transport_tx_buffer();
  (void)pvParameters;

  if (lwpkt_init(&protocol_packet, tx_buffer, rx_buffer) != lwpktOK)
  {
      vTaskDelete(NULL);
      return;
  }
  protocol_ready = 1U;

  for (;;)
  {
    lwpktr_t result;
    const uint32_t now_ms = (uint32_t)xTaskGetTickCount();

    (void)xSemaphoreTake(protocol_tx_mutex, portMAX_DELAY);
    do
    {
      result = lwpkt_process(&protocol_packet, now_ms);
      if (result == lwpktVALID)
      {
          protocol_dispatch_packet(&protocol_packet);
      }
    } while (result == lwpktVALID || result == lwpktERR || result == lwpktERRCRC ||
             result == lwpktERRSTOP || result == lwpktERRMEM);

    serial_transport_poll_tx();
    (void)xSemaphoreGive(protocol_tx_mutex);
    (void)ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(PROTOCOL_TASK_PERIOD_MS));
  }
}

/* This API is for FreeRTOS task context; it is not ISR-safe. */
int32_t serial_protocol_send(uint32_t command, const void *data, size_t length)
{
  lwpktr_t result;
  const BaseType_t protocol_task_owns_mutex =
      (xTaskGetCurrentTaskHandle() == ProtocolTask_Handle) ? pdTRUE : pdFALSE;

  if (protocol_ready == 0U || protocol_tx_mutex == NULL ||
      (protocol_task_owns_mutex == pdFALSE &&
       xSemaphoreTake(protocol_tx_mutex, pdMS_TO_TICKS(10U)) != pdTRUE))
  {
      return -1;
  }

  result = lwpkt_write(&protocol_packet, command, data, length);
  if (result == lwpktOK)
  {
      serial_transport_poll_tx();
  }
  if (protocol_task_owns_mutex == pdFALSE)
  {
      (void)xSemaphoreGive(protocol_tx_mutex);
  }
  return (result == lwpktOK) ? 0 : -(int32_t)result;
}
