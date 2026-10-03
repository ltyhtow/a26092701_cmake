# 安徽端 AI 代理执行任务指令 (Host Agent Instructions)

> **给安徽端 AI 的角色设定：**
> 你当前运行在**安徽的 Windows 主机**上（虚拟局域网 IP：`100.79.218.27`）。
> 这台电脑通过 USB 物理连接了 **ST-Link 仿真器**（连接 STM32C562 核心板）以及相关的 **虚拟串口/透传模块**。
> 远在**河南的开发机**需要通过虚拟局域网对这块 MCU 进行远程烧录、GDB 仿真以及串口遥测。
> 
> **你的核心任务：**
> 自动化检查本地硬件连接，并在后台启动两个常驻网络服务：
> 1. **GDB Server 服务**：监听 `0.0.0.0:3333`，负责把河南发来的 GDB 协议转给 ST-Link SWD 接口。
> 2. **串口网络桥服务**：监听 `0.0.0.0:8080`，负责把单片机的串口数据（115200波特率）双向透传给网络。
> 3. **开放本地 Windows 防火墙**：确保入站规则放行 3333 与 8080 端口。

---

## 任务执行清单（请按顺序执行）

### 任务 1：检测 ST-Link 与串口硬件挂载

在 PowerShell 中执行以下命令，确认 ST-Link 和 COM 口已被系统识别：

```powershell
# 1. 查找 ST-Link 硬件
Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match "STLink|ST-Link" } | Select-Object Name, DeviceID, Status

# 2. 查找 ST-Link 自带的虚拟串口号（或者外接的 USB-TTL 串口号）
Get-CimInstance Win32_SerialPort | Select-Object DeviceID, Description
```

*记录识别到的串口号（例如 `COM3` 或 `COM5`），后面的串口转发脚本需要用到。*

---

### 任务 2：启动 ST-Link GDB Server（端口 3333）

在本地定位并启动 `ST-LINK_gdbserver.exe`。它通常位于以下默认路径之一：
* `C:\ST\STM32CubeCLT\ST-LINK_gdbserver\bin\ST-LINK_gdbserver.exe`
* `C:\ST\STM32CubeProgrammer\bin\ST-LINK_gdbserver.exe`
* 或 STM32CubeIDE 安装目录下的 plugins 路径。

请先查找其绝对路径：
```powershell
$gdbserver = (Get-ChildItem -Path "C:\Program Files", "C:\Program Files (x86)", "C:\ST", "C:\Users" -Filter "ST-LINK_gdbserver.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1).FullName
Write-Host "Found ST-LINK GDB Server: $gdbserver"
```

在后台启动服务（必须包含 `-d` 允许非本地连接，`-e` 保持常驻等待重连）：
```powershell
# 假设 CubeProgrammer 安装在 C:\ST\STM32CubeProgrammer\bin
# -cp 参数指定 STM32CubeProgrammer 核心 DLL 目录
# -p 3333: 监听 3333 端口
# -d: 允许远程连接 (bind to 0.0.0.0)
# -e: 持续常驻模式
Start-Process -FilePath $gdbserver -ArgumentList "-p 3333 -d -e" -WindowStyle Minimized

# 如果机器上只有 OpenOCD，可用下述命令替代：
# Start-Process -FilePath "openocd.exe" -ArgumentList "-f interface/stlink.cfg -f target/stm32c5x.cfg -c ""bindto 0.0.0.0""" -WindowStyle Minimized
```

---

### 任务 3：创建并启动串口网络桥（端口 8080）

检查并安装 `pyserial`：
```powershell
python -m pip install pyserial --quiet
```

在本地工作目录（例如 `C:\Users\30496\serial_to_tcp.py`）写入以下脚本：

```python
import socket
import serial
import threading
import sys

# 请将 COM3 替换为 任务 1 中识别到的实际串口号
COM_PORT = 'COM3'
BAUD_RATE = 115200
TCP_PORT = 8080

try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=0.1)
    print(f"[*] 成功打开串口 {COM_PORT}, 波特率 {BAUD_RATE}")
except Exception as e:
    print(f"[-] 无法打开串口 {COM_PORT}: {e}")
    sys.exit(1)

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('0.0.0.0', TCP_PORT))
server.listen(1)
print(f"[*] 串口网络桥已启动，正在监听 0.0.0.0:{TCP_PORT} ...")

def serial_to_tcp(conn):
    while True:
        try:
            data = ser.read(ser.in_waiting or 1)
            if data:
                conn.sendall(data)
        except Exception:
            break

def tcp_to_serial(conn):
    while True:
        try:
            data = conn.recv(1024)
            if not data:
                break
            ser.write(data)
        except Exception:
            break

while True:
    print("[*] 等待河南客户端连接...")
    conn, addr = server.accept()
    print(f"[+] 河南客户端已连接: {addr}")
    
    t1 = threading.Thread(target=serial_to_tcp, args=(conn,), daemon=True)
    t1.start()
    tcp_to_serial(conn)
    conn.close()
    print("[-] 客户端断开，重新等待连接...")
```

后台启动该转发脚本：
```powershell
Start-Process -FilePath "python" -ArgumentList "C:\Users\30496\serial_to_tcp.py" -WindowStyle Minimized
```

---

### 任务 4：放行 Windows 防火墙

在管理员权限的 PowerShell 中执行，允许两个端口从虚拟局域网连入：
```powershell
New-NetFirewallRule -DisplayName "Remote-Debug-GDB-3333" -Direction Inbound -LocalPort 3333 -Protocol TCP -Action Allow -ErrorAction SilentlyContinue
New-NetFirewallRule -DisplayName "Remote-Debug-COM-8080" -Direction Inbound -LocalPort 8080 -Protocol TCP -Action Allow -ErrorAction SilentlyContinue
```

---

### 任务 5：自检与汇报

执行以下自检命令，确认两个端口已经处于 `LISTENING` 状态：

```powershell
Get-NetTCPConnection -LocalPort 3333, 8080 -ErrorAction SilentlyContinue | Select-Object LocalAddress, LocalPort, State
```

**预期输出：**
* `LocalPort 3333` $\rightarrow$ State: `Listen`
* `LocalPort 8080` $\rightarrow$ State: `Listen`

**完成汇报：**
当两个端口均处于 Listen 状态时，向用户确认：
> “安徽端服务已就绪！GDB Server（端口 3333）与串口网络桥（端口 8080）已在 `100.79.218.27` 上启动监听，防火墙已放行，请通知河南端发起连接测试。”
