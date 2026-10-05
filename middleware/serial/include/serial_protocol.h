#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LwPKT command values used by the application protocol. */
#define SERIAL_CMD_MOTION       0x01U
#define SERIAL_CMD_PID_CONFIG   0x02U
#define SERIAL_CMD_SYSTEM       0x03U
#define SERIAL_CMD_TELEMETRY    0x81U
#define SERIAL_CMD_PID_ACK      0x82U
#define SERIAL_CMD_RESPONSE     0x83U

#define SERIAL_PROTOCOL_VERSION 1U

typedef struct {
    uint8_t  version;
    uint8_t  sequence;
    uint8_t  reserved[2];
    int32_t  linear_q16_16;
    int32_t  yaw_q16_16;
    uint16_t timeout_ms;
    uint16_t flags;
} motion_command_t;

typedef struct {
    uint8_t  version;
    uint8_t  sequence;
    uint8_t  loop_id;
    uint8_t  reserved;
    int32_t  kp_q16_16;
    int32_t  ki_q16_16;
    int32_t  kd_q16_16;
    int32_t  integral_limit_q16_16;
} pid_config_command_t;

typedef struct {
    uint8_t version;
    uint8_t sequence;
    uint8_t action;
    uint8_t reserved;
} system_command_t;

typedef struct {
    uint8_t  version;
    uint8_t  sequence;
    uint16_t status_flags;
    int32_t  angle_q16_16;
    int32_t  gyro_q16_16;
    int32_t  left_speed_q16_16;
    int32_t  right_speed_q16_16;
    int16_t  left_pwm;
    int16_t  right_pwm;
    uint16_t battery_mv;
    uint16_t fault_code;
    uint32_t timestamp_ms;
} motion_telemetry_t;

_Static_assert(sizeof(motion_command_t) == 16U, "motion command wire size");
_Static_assert(sizeof(pid_config_command_t) == 20U, "pid command wire size");
_Static_assert(sizeof(system_command_t) == 4U, "system command wire size");
_Static_assert(sizeof(motion_telemetry_t) == 32U, "motion telemetry wire size");

/* Thread-safe packet submission API for FreeRTOS task context. */
int32_t serial_protocol_send(uint32_t command, const void *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_PROTOCOL_H */
