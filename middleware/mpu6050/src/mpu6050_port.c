#include "mpu6050_port.h"

#include "mx_i2c1.h"
#include "stm32_hal.h"

static mpu6050_handle_t mpu6050_handle;
static uint8_t mpu6050_ready;

static uint8_t mpu6050_iic_init(void) {
    return (mx_i2c1_i2c_gethandle() != NULL) ? 0U : 1U;
}

static uint8_t mpu6050_iic_deinit(void) {
    return 0U;
}

static uint8_t mpu6050_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
    return (HAL_I2C_MASTER_MemRead(mx_i2c1_i2c_gethandle(), (uint32_t)(addr >> 1U), reg,
                                   HAL_I2C_MEM_ADDR_8BIT, buf, len, 100U) == HAL_OK)
               ? 0U
               : 1U;
}

static uint8_t mpu6050_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
    return (HAL_I2C_MASTER_MemWrite(mx_i2c1_i2c_gethandle(), (uint32_t)(addr >> 1U), reg,
                                    HAL_I2C_MEM_ADDR_8BIT, buf, len, 100U) == HAL_OK)
               ? 0U
               : 1U;
}

static void mpu6050_delay_ms(uint32_t delay_ms) {
    HAL_Delay(delay_ms);
}

static void mpu6050_debug_print(const char *const fmt, ...) {
    (void)fmt;
}

static void mpu6050_receive_callback(uint8_t type) {
    (void)type;
}

static void mpu6050_dmp_tap_callback(uint8_t count, uint8_t direction) {
    (void)count;
    (void)direction;
}

static void mpu6050_dmp_orient_callback(uint8_t orientation) {
    (void)orientation;
}

uint8_t mpu6050_port_init(void) {
    uint8_t result;

    DRIVER_MPU6050_LINK_INIT(&mpu6050_handle, mpu6050_handle);
    DRIVER_MPU6050_LINK_IIC_INIT(&mpu6050_handle, mpu6050_iic_init);
    DRIVER_MPU6050_LINK_IIC_DEINIT(&mpu6050_handle, mpu6050_iic_deinit);
    DRIVER_MPU6050_LINK_IIC_READ(&mpu6050_handle, mpu6050_iic_read);
    DRIVER_MPU6050_LINK_IIC_WRITE(&mpu6050_handle, mpu6050_iic_write);
    DRIVER_MPU6050_LINK_DELAY_MS(&mpu6050_handle, mpu6050_delay_ms);
    DRIVER_MPU6050_LINK_DEBUG_PRINT(&mpu6050_handle, mpu6050_debug_print);
    DRIVER_MPU6050_LINK_RECEIVE_CALLBACK(&mpu6050_handle, mpu6050_receive_callback);
    mpu6050_handle.dmp_tap_callback = mpu6050_dmp_tap_callback;
    mpu6050_handle.dmp_orient_callback = mpu6050_dmp_orient_callback;

    mpu6050_handle.iic_addr = MPU6050_ADDRESS_AD0_LOW;
    result = mpu6050_init(&mpu6050_handle);
    if (result != 0U) {
        mpu6050_ready = 0U;
        return result;
    }

    result = mpu6050_set_clock_source(&mpu6050_handle, MPU6050_CLOCK_SOURCE_PLL_X_GYRO);
    if (result == 0U) {
        result = mpu6050_set_sample_rate_divider(&mpu6050_handle, 4U);
    }
    if (result == 0U) {
        result = mpu6050_set_low_pass_filter(&mpu6050_handle, MPU6050_LOW_PASS_FILTER_3);
    }
    if (result == 0U) {
        result = mpu6050_set_gyroscope_range(&mpu6050_handle, MPU6050_GYROSCOPE_RANGE_500DPS);
    }
    if (result == 0U) {
        result = mpu6050_set_accelerometer_range(&mpu6050_handle, MPU6050_ACCELEROMETER_RANGE_2G);
    }
    if (result != 0U) {
        (void)mpu6050_deinit(&mpu6050_handle);
        mpu6050_ready = 0U;
        return result;
    }
    mpu6050_ready = 1U;
    return 0U;
}

uint8_t mpu6050_port_read(mpu6050_sample_t *sample) {
    uint16_t length = 1U;

    if (sample == NULL || mpu6050_ready == 0U) {
        return 1U;
    }
    return mpu6050_read(&mpu6050_handle, &sample->accel_raw, &sample->accel_g,
                        &sample->gyro_raw, &sample->gyro_dps, &length);
}

uint8_t mpu6050_port_deinit(void) {
    if (mpu6050_ready == 0U) {
        return 0U;
    }
    mpu6050_ready = 0U;
    return mpu6050_deinit(&mpu6050_handle);
}
