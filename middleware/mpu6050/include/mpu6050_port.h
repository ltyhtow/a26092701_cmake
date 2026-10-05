#ifndef MPU6050_PORT_H
#define MPU6050_PORT_H

#include <stdint.h>
#include "driver_mpu6050.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t accel_raw[3];
    float accel_g[3];
    int16_t gyro_raw[3];
    float gyro_dps[3];
} mpu6050_sample_t;

typedef struct {
    uint8_t  address_8bit;
    uint8_t  who_am_i;
    uint16_t init_result;
    uint32_t hal_status;
    uint32_t hal_error_codes;
} mpu6050_diagnostics_t;

uint8_t mpu6050_port_init(void);
uint8_t mpu6050_port_read(mpu6050_sample_t *sample);
void mpu6050_port_get_diagnostics(mpu6050_diagnostics_t *diagnostics);
uint8_t mpu6050_port_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* MPU6050_PORT_H */
