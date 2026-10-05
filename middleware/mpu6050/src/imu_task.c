#include "imu_task.h"

#include "task.h"

#include <string.h>

#define IMU_TASK_STACK_DEPTH_WORDS 384U
#define IMU_TASK_PERIOD_MS 5U

static volatile uint32_t imu_init_failures;

static QueueHandle_t imu_sample_queue;

static void imu_task_entry(void *argument) {
    imu_sample_message_t message;
    mpu6050_sample_t sample;

    (void)argument;
    while (mpu6050_port_init() != 0U) {
        memset(&message, 0, sizeof(message));
        message.timestamp_ms = (uint32_t)xTaskGetTickCount();
        imu_init_failures++;
        message.error_code = (uint16_t)imu_init_failures;
        (void)xQueueOverwrite(imu_sample_queue, &message);
        vTaskDelay(pdMS_TO_TICKS(500U));
    }

    for (;;) {
        memset(&message, 0, sizeof(message));
        message.timestamp_ms = (uint32_t)xTaskGetTickCount();
        if (mpu6050_port_read(&sample) == 0U) {
            message.status_flags = 0x0001U;
            message.sample = sample;
        } else {
            message.error_code = 1U;
        }
        (void)xQueueOverwrite(imu_sample_queue, &message);
        vTaskDelay(pdMS_TO_TICKS(IMU_TASK_PERIOD_MS));
    }
}

BaseType_t imu_task_start(QueueHandle_t sample_queue, TaskHandle_t *task_handle) {
    if (sample_queue == NULL || task_handle == NULL) {
        return pdFAIL;
    }
    imu_sample_queue = sample_queue;
    return xTaskCreate(imu_task_entry, "IMU", IMU_TASK_STACK_DEPTH_WORDS,
                       NULL, 3U, task_handle);
}
