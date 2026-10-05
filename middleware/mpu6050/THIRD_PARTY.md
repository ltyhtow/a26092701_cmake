# LibDriver MPU6050

The `third_party/libdriver` directory contains the MPU6050 driver from
`libdriver/mpu6050`, released under the MIT license.

The project-specific adapter in `src/mpu6050_port.c` connects LibDriver's
platform-independent I2C callbacks to the generated STM32C5 HAL I2C1 handle.
The driver is used for register configuration and raw accel/gyro sampling;
the balance controller and attitude estimator remain application code.

The imported LibDriver source files retain their original MIT license headers.

LibDriver's address constants use the left-shifted device address form
(`0xD0`/`0xD2`). The STM32C5 HAL master memory APIs also document that the
7-bit datasheet address must be shifted left before the call, so the adapter
passes LibDriver's address through unchanged.
