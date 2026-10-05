# a26092701 两轮平衡车项目交接记录

更新时间：2026-10-05 03:08

这份文档给后续 agent 使用，记录当前 STM32C562CET6 两轮平衡车工程、TIM8 电机测试、F411 PWM 监测器以及已经得到的硬件证据。

## 工程位置

- CubeMX2 配置：`C:\Users\30496\CLionProjects\a26092701.ioc2`
- C5 生成工程：`C:\Users\30496\CLionProjects\a26092701_cmake`
- C5 电机测试任务：`generated\middleware\mx_freertos_app.c`
- C5 TIM8 初始化：`generated\hal\mx_tim8.c`
- F411 监测工程：`C:\Users\30496\CLionProjects\f411_pwm_probe`
- F411 串口版 HEX：`f411_pwm_probe\build_serial\f411_pwm_probe.hex`
- F411 串口版 BIN：`f411_pwm_probe\build_serial\f411_pwm_probe.bin`

C5 工程当前工作树中有用户/生成文件修改，不能使用 `git reset --hard` 或覆盖无关修改。

## 已确认的 C5 配置

### TIM8 电机 PWM

| 功能 | C5 引脚 | 复用 | 标签 |
|---|---|---|---|
| 电机 1 IN1 | PB10 | AF2 / TIM8_CH1 | MOTOR1_IN1 |
| 电机 1 IN2 | PB13 | AF2 / TIM8_CH2 | MOTOR1_IN2 |
| 电机 2 IN1 | PB12 | AF2 / TIM8_CH3 | MOTOR2_IN1 |
| 电机 2 IN2 | PB6 | AF13 / TIM8_CH4 | MOTOR2_IN2 |

当前 `mx_tim8.c` 已包含 GPIOB 时钟、PB10/PB13/PB12 的 AF2 初始化和 PB6 的 AF13 初始化。之前的 CubeMX2 问题是通道虽然存在，但 `use_channel/chx_gpio` 没有打开，导致生成 `No GPIO configuration required for TIM8`；现在已通过 CubeMX2 配置修正。

TIM8 当前参数：

- C5 系统时钟：144 MHz
- 预分频：0
- 自动重装值：`0x1C1F` = 7199
- 目标 PWM：20 kHz
- 周期：约 50 us

TIM8 是高级定时器，测试任务中显式调用了 `HAL_TIM_BREAK_EnableMainOutput(tim8)`，否则 MOE 可能未打开。

### 其他外设

- TIM2 编码器 1：PA5 `TIM2_CH1`、PB3 `TIM2_CH2`，X4 编码器模式，输入滤波 N8。
- TIM5 编码器 2：PA0 `TIM5_CH1`、PA1 `TIM5_CH2`，X4 编码器模式，输入滤波 N8。
- I2C1：PA8 SCL AF9、PB7 SDA AF4，400 kHz；需要外部上拉。
- MPU6050 INT：PB1，上拉，EXTI1，上升沿中断；AD0 计划接 GND，地址为 0x68。
- C5 USART1：PA10 RX AF7、PA15 TX AF11，115200 8N1，已配置 DMA/IRQ。
- 电池 ADC：PB0 / ADC2_IN6，AT8236 电压采样比例为 VIN/11。
- SWD：PA13/PA14；板载指示灯：PA3。
- FreeRTOS FPU 已在 `.ioc2` 中开启，生成的 `FreeRTOSConfig.h` 为 `configENABLE_FPU 1U`。

## 历史 C5 电机测试任务（已清理）

历史文件：`generated\middleware\mx_freertos_app.c`

该测试任务已在 2026-10-05 清理。当前 C5 应用只保留 Task1 闪灯任务，不再启动 TIM8 电机 PWM、不再配置 PB10/PB13/PB12/PB6 为普通 GPIO 诊断输出，也不再执行电机方向测试。

关键参数：

```c
#define MOTOR_TEST_DUTY   2520U
#define MOTOR_TEST_HOLD_MS 1000U
#define MOTOR_TEST_PAUSE_MS 500U
```

`2520 / 7200` 约等于 35% 占空比。任务启动四个 TIM8 OC 通道、启动 TIM8、打开 MOE，然后依次执行：

1. 电机 1 IN1 PWM，IN2 为 0，持续 1 s
2. 电机 1 IN2 PWM，IN1 为 0，持续 1 s
3. 电机 2 IN1 PWM，IN2 为 0，持续 1 s
4. 电机 2 IN2 PWM，IN1 为 0，持续 1 s
5. 两个电机的 IN1 同时 PWM，持续 1 s
6. 每个阶段后暂停 500 ms，最后清零并停止 TIM8

因此完整测试窗口只有约 7.5 s。测试结束后，任务只闪烁 PA3，TIM8 输出已经停止。用 F411 监测时必须在 C5 复位后立即开始记录，否则读到全零是正常的。

启动失败时 PA3 快速闪烁（100 ms）；启动成功并完成测试后 PA3 约 500 ms 翻转一次。

## F411 PWM 监测器

工程：`C:\Users\30496\CLionProjects\f411_pwm_probe`

WeAct Black Pill F411CEU6 使用 TIM2 输入捕获，PA0~PA3 分别监测 C5 四路输出：

| C5 | F411 |
|---|---|
| PB10 / TIM8_CH1 | PA0 / TIM2_CH1 |
| PB13 / TIM8_CH2 | PA1 / TIM2_CH2 |
| PB12 / TIM8_CH3 | PA2 / TIM2_CH3 |
| PB6 / TIM8_CH4 | PA3 / TIM2_CH4 |
| GND | GND |

不要把 AT8236 电机电源或电机端子接到 F411。F411 只接 3.3 V 逻辑信号和公共地。

串口输出：

| 串口模块 | F411 |
|---|---|
| RX | PA9 / USART1_TX |
| TX | PA10 / USART1_RX，可不接 |
| GND | GND |
| 3.3V | F411 3.3V，仅在模块确实为 3.3 V TTL 时连接 |

串口参数为 115200、8 数据位、无校验、1 停止位。串口版固件同时输出普通文本和数字专用帧。数字帧格式为：

```text
MM PPPP HHHH PPPP HHHH PPPP HHHH PPPP HHHH
```

实际发送时没有空格：

- `MM`：有效通道掩码
- 每路 4 位周期，单位 us
- 每路 4 位高电平时间，单位 us

例如 `010000500017...` 表示 CH1 有效、周期约 50 us、高电平约 17 us。

2026-10-04 19:16 已修正 F411 监测器的报告快照：`Core\\Src\\main.c` 在关中断临界区内复制四路周期、高电平、时间戳和有效掩码，再生成两种串口帧，避免中断更新期间出现掩码与数字数据不一致。已重新构建并刷新：

- `f411_pwm_probe\\build_serial\\f411_pwm_probe.elf`
- `f411_pwm_probe\\build_serial\\f411_pwm_probe.hex`
- `f411_pwm_probe\\build_serial\\f411_pwm_probe.bin`

F411 串口工程最后一次成功构建：

```text
text 27228, data 96, bss 20968
```

## 蓝牙串口 HEX 的解析结果

蓝牙模块目前会把 ASCII 高位弄乱，但低 6 位仍可用于还原。对收到的每个字节 `b`，当前数据可用下面的还原式：

```text
decoded = (b & 0x3F) | ((b & 0x80) >> 1)
```

例如：

```text
0x90 -> 'P' (0x50)
0x97 -> 'W' (0x57)
0x8D -> 'M' (0x4D)
0x70 -> '0' (0x30)
```

用户在 18:53:31 的一帧中，换行后的数字帧还原为近似：

```text
0000470017000000000000000000000000000000
```

含义是：

- 掩码读到 `00`；
- CH1 曾捕获周期约 47 us、高电平约 17 us；
- CH2~CH4 为 0。

47 us 对应约 21.3 kHz，17/47 约 36%，与 C5 配置的 20 kHz、35% 接近。这是目前最重要的硬件证据：**PB10/TIM8_CH1 确实曾经输出过 PWM**。

18:53:46、18:53:47、18:53:48、18:53:50 等后续帧的数字部分主要是全零，符合 C5 测试任务已结束并停止 TIM8 的行为。

F411 监测器已改为在关中断临界区内复制四路周期、高电平、时间戳和有效掩码，再生成两种串口帧，避免报告中的掩码与数字数据瞬间不一致。

## 当前结论

当前代码状态：C5 已恢复为只有一个 FreeRTOS 闪灯任务；电机测试代码和 GPIO 诊断代码已清理。TIM8 外设生成配置仍保留在工程中，供后续正式电机控制使用。

已经证明：

1. CubeMX2 中 TIM8 四路 GPIO 绑定已修正。
2. 生成的 TIM8 初始化包含 PB10/PB13/PB12/PB6 的复用输出配置。
3. C5 测试任务执行到了 FreeRTOS 任务体，且显式启动了 TIM8/MOE。
4. F411 监测到过 PB10 的约 20 kHz PWM。

尚未证明：

1. PB13、PB12、PB6 三路是否在各自测试阶段输出。
2. AT8236 是否收到正确的 AIN1/AIN2/BIN1/BIN2 信号。
3. AT8236 电机电源、逻辑地和 MC310 电机线是否接对。
4. 电机不转是否由驱动板电源/接线/使能条件造成。

硬件诊断结论：此前 PB10 无电压的原因是面包板故障。更换面包板后，使用 100% PWM 固件测得约 3.3 V，证明 C5 输出链路正常。

## 下一步建议

1. 如需重新验证电机，先从 Git 基线恢复/复制独立的电机测试任务，再烧录；当前固件只闪灯，不会输出电机 PWM。
2. 先只连接 PB10→PA0，确认第一阶段出现约 47~50 us 周期；再分别连接 PB13→PA1、PB12→PA2、PB6→PA3。
3. 为避免 7.5 s 窗口太短，临时把 C5 测试改为只保持一路 PWM 10~30 s，或循环测试，不要同时改动多个变量。
4. 确认 AT8236：C5 GND 与驱动板 GND 共地；PB10/PB13 接 AIN1/AIN2；PB12/PB6 接 BIN1/BIN2；驱动板电机电源符合 MC310 额定 7.4 V；MC310 电机端子接驱动板 MOTOR1/MOTOR2。
5. 若四路 PWM 都能被 F411 捕获而电机仍不转，停止继续修改 CubeMX 配置，转查 AT8236 电源、地、输入端子和电机端子。
6. 后续生成 CubeMX2 代码前，先备份 `mx_freertos_app.c` 中的自定义测试任务；生成操作可能覆盖该文件。

本次主机检查（2026-10-04 19:16-19:19）未发现本机 USB ST-Link，也未找到 `ST-LINK_gdbserver.exe`；系统仅列出 COM1 和 COM3/COM5/COM6 蓝牙串口，3333/8080 均未监听。因此本机无法代替安徽端完成烧录、GDB 或串口网络桥验证；需要在连接实际 C5/F411 硬件的主机上执行上面的烧录和采集步骤。

## 构建命令

C5：

```powershell
& "C:\ST\STM32CubeCLT_1.20.0\CMake\bin\cmake.exe" --build --preset debug_GCC_STM32C562CET6
```

F411：

```powershell
& "C:\ST\STM32CubeCLT_1.20.0\CMake\bin\cmake.exe" --build "C:\Users\30496\CLionProjects\f411_pwm_probe\build_serial"
```

F411 使用 USB DFU 烧录时：

```powershell
& "C:\ST\STM32CubeCLT_1.20.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=USB1 -w "C:\Users\30496\CLionProjects\f411_pwm_probe\build_serial\f411_pwm_probe.hex" -v -rst
```

若使用 ST-Link，将 `port=USB1` 改成 `port=SWD`。
