#include "imu_task.h"

#include "task.h"

#define IMU_TASK_STACK_DEPTH_WORDS 384U
#define IMU_TASK_PERIOD_MS 5U

static volatile uint32_t imu_init_failures;

static QueueHandle_t imu_sample_queue;

static void imu_task_entry(void *argument) {
    mpu6050_sample_t sample;

    (void)argument;
    while (mpu6050_port_init() != 0U) {
        imu_init_failures++;
        vTaskDelay(pdMS_TO_TICKS(500U));
    }

    for (;;) {
        if (mpu6050_port_read(&sample) == 0U) {
            imu_sample_message_t message;
            message.timestamp_ms = (uint32_t)xTaskGetTickCount();
            message.sample = sample;
            (void)xQueueOverwrite(imu_sample_queue, &message);
        }
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
