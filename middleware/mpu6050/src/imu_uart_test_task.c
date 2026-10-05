#include "imu_uart_test_task.h"

#include "imu_task.h"
#include "serial_protocol.h"

#include <string.h>

#define IMU_UART_TEST_STACK_DEPTH_WORDS 256U
#define IMU_UART_TEST_PERIOD_MS         50U

static QueueHandle_t imu_uart_source_queue;

static void imu_uart_test_task_entry(void *argument) {
    imu_sample_message_t message;
    imu_telemetry_t telemetry;
    uint8_t sequence = 0U;

    (void)argument;
    for (;;) {
        if (xQueueReceive(imu_uart_source_queue, &message,
                          pdMS_TO_TICKS(IMU_UART_TEST_PERIOD_MS)) != pdTRUE) {
            memset(&message, 0, sizeof(message));
            message.timestamp_ms = (uint32_t)xTaskGetTickCount();
            message.error_code = 0xFFFFU;
        }

        memset(&telemetry, 0, sizeof(telemetry));
        telemetry.version = SERIAL_PROTOCOL_VERSION;
        telemetry.sequence = sequence++;
        telemetry.status_flags = message.status_flags;
        memcpy(telemetry.accel_raw, message.sample.accel_raw,
               sizeof(telemetry.accel_raw));
        memcpy(telemetry.gyro_raw, message.sample.gyro_raw,
               sizeof(telemetry.gyro_raw));
        telemetry.error_code = message.error_code;
        telemetry.timestamp_ms = message.timestamp_ms;
        (void)serial_protocol_send(SERIAL_CMD_IMU_STATUS,
                                   &telemetry, sizeof(telemetry));
    }
}

BaseType_t imu_uart_test_task_start(QueueHandle_t sample_queue,
                                    TaskHandle_t *task_handle) {
    if (sample_queue == NULL || task_handle == NULL) {
        return pdFAIL;
    }
    imu_uart_source_queue = sample_queue;
    return xTaskCreate(imu_uart_test_task_entry, "IMUTx",
                       IMU_UART_TEST_STACK_DEPTH_WORDS, NULL, 1U, task_handle);
}
