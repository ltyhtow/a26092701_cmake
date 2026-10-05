# LibDriver MPU6050

The `third_party/libdriver` directory contains the MPU6050 driver from
`libdriver/mpu6050`, released under the MIT license.

The project-specific adapter in `src/mpu6050_port.c` connects LibDriver's
platform-independent I2C callbacks to the generated STM32C5 HAL I2C1 handle.
The driver is used for register configuration and raw accel/gyro sampling;
the balance controller and attitude estimator remain application code.

The imported LibDriver source files retain their original MIT license headers.

LibDriver's address constants use the 8-bit bus form (`0xD0`/`0xD2`), while
the STM32C5 HAL expects a 7-bit target address. The adapter converts the
address with `addr >> 1` before every HAL transaction.
