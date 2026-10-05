# file-format: 1.0.0
cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

include(cmake/target.cmake)
include(cmake/${TOOLCHAIN_FILE})

project(a26092701)
add_executable(a26092701)
include(cmake/files.cmake)
include(cmake/flags.cmake)
include(cmake/components.cmake)

# Stubs to be able to add compiler flag overrides, e.g. -Os
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  middleware/serial/src/serial_transport.c
  middleware/serial/third_party/lwrb/src/lwrb/lwrb.c
  middleware/serial/third_party/lwpkt/src/lwpkt/lwpkt.c
  middleware/mpu6050/src/mpu6050_port.c
  middleware/mpu6050/src/imu_task.c
  middleware/mpu6050/src/imu_uart_test_task.c
  middleware/mpu6050/third_party/libdriver/src/driver_mpu6050.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
  middleware/serial/include
  middleware/serial/third_party/lwrb/src/include
  middleware/serial/third_party/lwpkt/src/include
  middleware/mpu6050/include
  middleware/mpu6050/third_party/libdriver/src
  middleware/mpu6050/third_party/libdriver/interface
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
  # Add additional defined symbols here
)

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC
  # Add additional compiler flags here
)

target_link_options(${CMAKE_PROJECT_NAME} PUBLIC
  # Add additional linker flags here
)

target_link_directories(${CMAKE_PROJECT_NAME} PUBLIC
  # Add additional linker search directories here
)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
  # Add additional linker libraries here
)
