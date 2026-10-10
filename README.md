# 设备数据平台（Device Platform）

一个"设备 — 平台"数据链路的最小完整实现：

- **设备端（C）**：模拟设备，按固定间隔通过 HTTP 向平台上报
- **平台端（Python / FastAPI + SQLite）**：接收上报、入库、提供查询接口
- **网页面板（HTML + JS）**：定时刷新，实时显示最新状态与历史记录

## 目录结构

| 路径 | 说明 |
|---|---|
| `device/net.h` / `net.c` | 网络层：只负责发一个 HTTP POST，不认识"设备" |
| `device/device.h` / `device.c` | 设备层：设备的数据与行为，不认识 socket |
| `device/main.c` | 组装层：初始化设备 + 循环上报 |
| `fastP/main.py` | 平台端：FastAPI + SQLite |
| `temple/first.html` | 网页面板 |

## 接口约定

| 项 | 值 |
|---|---|
| 方法 / 路径 | `POST /api/report` |
| 查询参数 | `device`（设备名，必填）、`ts`（时间戳，可选） |
| 成功响应 | `200` + JSON |
| 查询最新状态 | `GET /api/status` |
| 查询历史 | `GET /api/history?limit=10` |
| 网页面板 | `GET /panel` |

## 怎么跑

### 1. 启动平台（Windows）

```bash
cd fastP
python -m uvicorn main:app --reload --port 8080
```

> 端口用 8080：本机 8000 会被 Windows 拒绝（WinError 10013）

### 2. 编译并运行设备端（需要 mingw gcc）

```bash
cd device
gcc -Wall main.c device.c net.c -o device.exe -lws2_32
device.exe C-device-01 2000
```

### 3. 打开面板

浏览器访问 http://127.0.0.1:8080/panel

## 技术要点

- 设备端手写 HTTP 报文（请求行 + 请求头 + 空行），不依赖任何 HTTP 库
- 分层设计：网络层与设备层解耦，替换传输方式（串口 / MQTT）只需改网络层
- 端口字节序转换（htons）、Windows Socket 初始化（WSAStartup）
- Windows 下编译需链接 `-lws2_32`

## 待办

- [ ] 上报数值（温度 / 电量），而不是只有心跳
- [ ] 传输层改为函数指针，实现可替换的传输方式
- [ ] 加入 CRC16 校验
- [ ] 移植到 Linux（去掉 winsock 依赖）