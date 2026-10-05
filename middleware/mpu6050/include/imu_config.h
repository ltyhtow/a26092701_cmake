#ifndef IMU_CONFIG_H
#define IMU_CONFIG_H

/* Keep the raw sampler active while leaving bring-up UART traffic opt-in. */
#define IMU_SAMPLE_PERIOD_MS     5U
#define IMU_UART_TEST_ENABLED    0U
#define IMU_UART_TEST_PERIOD_MS  50U

#endif /* IMU_CONFIG_H */
