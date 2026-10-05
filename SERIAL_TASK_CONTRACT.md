# 串口协议与任务接口契约

本文件先定义业务接口，再移植 `lwrb` 和 `lwpkt`。协议库只负责字节缓冲、组帧、拆帧和 CRC；运动控制任务不直接访问 UART，也不依赖协议库的内部状态。

## 1. 数据流

```text
USART1 RX DMA/ISR
        |
        v
   RX lwrb 字节管道
        |
        v
   lwpkt 协议任务
        |
        +--> motion_cmd_queue --> 运动/平衡控制任务
        +--> pid_config_queue  --> PID 参数管理任务
        +--> system_cmd_queue  --> 系统控制任务

遥测/应答任务
        |
        v
   lwpkt TX --> TX lwrb --> USART1 TX DMA
```

UART 中断/DMA 回调只搬运字节或通知任务，不解析结构体、不执行 PID、不控制电机。

## 2. LwPKT 配置

采用 LwPKT 的原生帧格式，不额外实现 `CFFC` 状态机：

```text
START [CMD] LEN DATA CRC STOP
```

建议配置：

```c
#define LWPKT_CFG_MAX_DATA_LEN 64
#define LWPKT_CFG_USE_ADDR     LWPKT_OFF
#define LWPKT_CFG_USE_FLAGS    LWPKT_OFF
#define LWPKT_CFG_USE_CMD      LWPKT_ON_STATIC
#define LWPKT_CFG_USE_CRC      LWPKT_ON_STATIC
#define LWPKT_CFG_CRC32        LWPKT_OFF
```

`CMD` 是业务消息类型，`LEN` 由 LwPKT 自动编码，CRC 由 LwPKT 自动生成和校验。串口任务只接受 `lwpktVALID` 的完整帧。

## 3. 命令编号

命令值按方向划分，避免把控制命令误当成遥测数据：

| CMD | 方向 | 含义 |
|---:|---|---|
| `0x01` | 上位机 -> C5 | 运动目标 |
| `0x02` | 上位机 -> C5 | PID 参数更新 |
| `0x03` | 上位机 -> C5 | 系统控制 |
| `0x81` | C5 -> 上位机 | 运动状态遥测 |
| `0x82` | C5 -> 上位机 | PID 参数回读/确认 |
| `0x83` | C5 -> 上位机 | 应答或错误 |

未知 CMD、长度不匹配、数值越界的帧必须丢弃，并可发送 `0x83` 错误应答；不能把不完整数据送入控制任务。

## 4. 线协议结构体

以下结构体是业务层的“线格式”，只使用固定宽度整数，不直接使用指针、`bool`、枚举或未明确大小的类型。发送和接收都通过 `memcpy` 在字节数组与结构体之间转换。

### 4.1 公共头

每个负载包含：

```c
typedef struct {
    uint8_t  version;       /* 当前为 1 */
    uint8_t  sequence;      /* 发送方递增序号 */
} protocol_payload_header_t;
```

### 4.2 运动目标：CMD `0x01`

```c
typedef struct {
    uint8_t  version;
    uint8_t  sequence;
    uint8_t  reserved[2];
    int32_t  linear_q16_16;  /* 车体前后目标速度，m/s */
    int32_t  yaw_q16_16;     /* 目标转动速度，rad/s */
    uint16_t timeout_ms;     /* 超时后控制任务必须停车 */
    uint16_t flags;          /* bit0: enable, bit1: clear_fault */
} motion_command_t;
```

负载固定为 16 字节。保留字段用于消除编译器对齐差异，发送前应将其清零。`timeout_ms` 应由控制任务监督，通信中断或上位机失联时输出必须进入安全状态。

### 4.3 PID 参数：CMD `0x02`

```c
typedef struct {
    uint8_t  version;
    uint8_t  sequence;
    uint8_t  loop_id;        /* 0: 角度环, 1: 速度环, 2: 角速度环 */
    uint8_t  reserved;
    int32_t  kp_q16_16;
    int32_t  ki_q16_16;
    int32_t  kd_q16_16;
    int32_t  integral_limit_q16_16;
} pid_config_command_t;
```

负载固定为 20 字节。协议任务只负责校验格式并入队；PID 任务在控制周期边界应用新参数，并检查允许范围。

### 4.4 系统控制：CMD `0x03`

```c
typedef struct {
    uint8_t version;
    uint8_t sequence;
    uint8_t action;          /* 1: enable, 2: disable, 3: clear fault, 4: reboot */
    uint8_t reserved;
} system_command_t;
```

`reboot` 等危险动作必须在系统任务中再次确认，不能由串口任务直接执行。

### 4.5 运动遥测：CMD `0x81`

```c
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
```

负载固定为 32 字节。`timestamp_ms` 用于上位机判断数据新鲜度；遥测发送失败不能阻塞控制环。

## 5. FreeRTOS 队列接口

协议任务解析成功后按 CMD 分发到以下队列：

```c
extern QueueHandle_t motion_command_queue;
extern QueueHandle_t pid_config_queue;
extern QueueHandle_t system_command_queue;
```

建议队列深度：

```text
motion_command_queue: 1
pid_config_queue:     2
system_command_queue: 4
```

消息是值拷贝，不传递指向 `lwpkt_t.data` 的指针。因为下一次解析会覆盖 `lwpkt_t.data`，保存指针会产生悬空数据。

队列满时：

- 运动目标：保留最新值，可以丢弃旧值；
- PID 参数：报告溢出，不静默覆盖；
- 系统控制：报告错误，不重复执行危险命令。

## 6. 任务职责

### 串口协议任务

- 从 RX `lwrb` 取字节并调用 `lwpkt_process()`；
- 只接受 `lwpktVALID`；
- 检查 CMD、负载长度、版本和数值范围；
- 将结构体值拷贝到对应队列；
- 处理超时、CRC 错误和未知命令，并累计协议统计；
- 从 TX `lwrb` 驱动 UART DMA。

其他 FreeRTOS 任务发送遥测或应答时调用 `serial_protocol_send()`；该函数是任务上下文接口，不可在 ISR 中调用，并返回明确的 `serial_protocol_result_t` 错误域。

### 运动/平衡控制任务

- 按固定控制周期运行；
- 消费最新 `motion_command_t`；
- 读取编码器、MPU6050 和电池状态；
- 计算 PID 并更新 TIM8 PWM；
- 检查运动目标超时、姿态越界和故障状态；
- 故障时关闭电机输出。

### PID 参数任务

- 消费 `pid_config_command_t`；
- 校验 `loop_id` 和参数范围；
- 在控制周期边界原子替换参数；
- 回传 `0x82` 确认或 `0x83` 错误。

## 6.1 IMU 数据接口

`IMU` 任务以 200 Hz 调用 LibDriver MPU6050 适配层，向单槽队列写入最新的 `imu_sample_message_t`：

```c
extern QueueHandle_t imu_sample_queue;

typedef struct {
    uint32_t timestamp_ms;
    mpu6050_sample_t sample;
} imu_sample_message_t;
```

该任务只负责 I2C 设备初始化和原始加速度/陀螺仪采样，不负责姿态融合、PID 或电机输出。MPU6050 不在线时任务周期性重试初始化，控制层应把没有新样本视为传感器故障。

## 7. 字节序与 ABI 规则

为了后续上位机兼容，必须明确规定：

- 所有多字节整数使用 little-endian；
- `q16_16 = 实数 * 65536`，采用有符号 `int32_t`；
- 不发送 C 结构体的 padding，发送前应确认 `sizeof`，必要时逐字段编码；
- `version` 变化时允许增加新命令，不改变旧命令已有字段的含义；
- 每种 CMD 的长度必须严格匹配，不能只判断“至少够长”。
- 线格式结构体必须使用显式保留字段或逐字段编码，不能依赖编译器自动插入的 padding；添加结构体后应使用 `_Static_assert(sizeof(type) == expected, "wire size")` 固定大小。

建议在公共协议头文件中固定这些尺寸：

```c
_Static_assert(sizeof(motion_command_t) == 16, "motion command wire size");
_Static_assert(sizeof(pid_config_command_t) == 20, "pid command wire size");
_Static_assert(sizeof(system_command_t) == 4, "system command wire size");
_Static_assert(sizeof(motion_telemetry_t) == 32, "motion telemetry wire size");
```

## 8. 移植顺序

1. 将 `lwrb` 和 `lwpkt` 源码纳入工程 CMake；
2. 添加 `lwpkt_opts.h`，按本文件配置；
3. 建立 RX/TX 静态环形缓冲区；
4. 实现 USART1 DMA 与环形缓冲的字节搬运；
5. 实现协议任务和 `lwpkt_process()`；
6. 先用 `0x01` 运动目标做回环测试；
7. 再接入 PID 队列和控制任务；
8. 最后启用遥测发送和故障应答。
