#ifndef IMU_TASK_H
#define IMU_TASK_H

#include "FreeRTOS.h"
#include "queue.h"
#include "mpu6050_port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t timestamp_ms;
    mpu6050_sample_t sample;
} imu_sample_message_t;

BaseType_t imu_task_start(QueueHandle_t sample_queue, TaskHandle_t *task_handle);

#ifdef __cplusplus
}
#endif

#endif /* IMU_TASK_H */
