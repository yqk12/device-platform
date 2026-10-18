# ESP32 设备端（ESP-IDF / C）

用 **C + ESP-IDF** 写的 ESP32 固件，作为「设备数据平台」的**真硬件设备端**。
与 [PC 上的 C 设备模拟器](../device/) 使用**同一套 HTTP 协议**上报到同一个平台。

```
ESP32（本目录 / C）  ──WiFi──HTTP POST──►  平台（../fastP / Python FastAPI）
PC C 模拟器（../device / C）──HTTP POST──►  同一个平台、同一份数据库
```

## 硬件

| 项 | 值 |
|---|---|
| 芯片 | ESP32-D0WD-V3（ESP32-WROOM-32 模块，4MB Flash） |
| 烧录方式 | USB 转串口 CH340 → **UART**（不是 JTAG） |
| 板载 LED | GPIO2 |
| WiFi | **仅支持 2.4GHz**（5G 连不上） |
| 串口 | 115200，退出监视器按 `Ctrl+]` |

## 环境

- ESP-IDF **v6.1**（Windows），target `esp32`
- 工具：`idf.py`（build / flash / monitor）

## 功能进度

- [x] **GPIO 输出**：GPIO2 翻转驱动板载 LED，`vTaskDelay(pdMS_TO_TICKS(400))` 控制节奏
- [x] **WiFi STA 连接**：事件回调 + FreeRTOS 事件组（`xEventGroupWaitBits`）等待联网
- [x] **HTTP 上报**：`esp_http_client` 每 5 秒 `POST /api/report?device=esp32-01`
- [ ] 采集真实传感器数据（替代固定值）
- [ ] 断线重连 + 本地缓存补传

## 快速开始

### 1. 填配置（**不要把密码写进 main.c 再提交**）

```bash
copy main\secrets.h.example main\secrets.h
```

编辑 `main\secrets.h`：

```c
#define WIFI_SSID   "你的2.4G热点"
#define WIFI_PASS   "你的密码"
#define SERVER_IP   "电脑的局域网IP"     /* 运行平台那台电脑 */
```

`secrets.h` 已在 `.gitignore` 里，**不会进仓库**。

### 2. 编译烧录

```bash
idf.py set-target esp32
idf.py build
idf.py -p COM11 flash monitor
```

烧录卡在 `Connecting....` → 按住板子 **BOOT** 键再试。

### 3. 平台要能被 ESP32 访问

平台端必须监听所有网卡（不是 127.0.0.1），并放行防火墙：

```powershell
# main.py: uvicorn.run(..., host="0.0.0.0", port=8080)
New-NetFirewallRule -DisplayName "ESP32-Platform-8080" -Direction Inbound -Protocol TCP -LocalPort 8080 -Action Allow -Profile Private
```

ESP32 和电脑必须在**同一个 WiFi（热点）**下。

## 学到的知识点

| 概念 | 在本项目里的体现 |
|---|---|
| `app_main()` | ESP-IDF 入口，由 FreeRTOS 的 main_task 调用（不是 `main`） |
| **位掩码配引脚** | `gpio_config_t.pin_bit_mask = (1ULL << pin)` —— 40 个引脚装不进 32 位 int |
| **tick 与毫秒** | FreeRTOS 1 tick = 10ms（默认），`pdMS_TO_TICKS(500)` = 50 tick |
| **回调 / 控制反转** | `esp_event_handler_instance_register(..., on_wifi_event, ...)`：把函数交给框架，框架在事件发生时调用 |
| **事件组** | 回调里 `xEventGroupSetBits`"举旗"，主任务 `xEventGroupWaitBits`"睡觉等旗" |
| **异步两步** | 连上 AP（`WIFI_EVENT_STA_CONNECTED`）≠ 能通信，必须等 `IP_EVENT_STA_GOT_IP`（DHCP 分配） |
| **句柄** | `EventGroupHandle_t`、`esp_http_client_handle_t` 都是"资源的遥控器"（不透明指针）|
| **串口独占** | 同一时刻只能一个程序占用；Flash 和 Monitor 不能同时开 |
| **回调里不做耗时操作** | 回调跑在事件循环任务里，阻塞它会堵住所有事件 |

## 目录结构

```
esp32/
├── CMakeLists.txt        # 顶层：project(esp32_device)
├── sdkconfig             # 构建配置（target/分区/Flash），要提交
├── main/
│   ├── CMakeLists.txt
│   ├── main.c            # 全部逻辑：GPIO + WiFi + HTTP
│   └── secrets.h.example # 配置模板（真正的 secrets.h 不进 git）
└── README.md
```
