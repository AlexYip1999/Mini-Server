# Mini Server Test Client

这是一个用于测试 Mini Server 功能的 C++ 客户端程序。

## 功能特性

- 完整的 HTTP 客户端实现（不依赖第三方库）
- 支持所有服务器端点的测试
- 详细的测试报告和结果统计
- 错误处理和异常情况测试
- 跨平台支持（Windows/Linux）

## 测试覆盖

测试客户端会验证以下功能：

1. **健康检查** - `GET /ping`
2. **服务列表** - `GET /services`
3. **Echo 服务** - `POST /service/echo`
4. **大写转换服务** - `POST /service/upper`
5. **字符串反转服务** - `POST /service/reverse`
6. **字符串长度服务** - `POST /service/length`
7. **错误处理测试**：
   - 不存在的服务（404错误）
   - 无效的HTTP方法（405错误）

## 构建说明

### 使用批处理脚本（推荐）

```batch
# Windows 下直接运行构建脚本
build_and_run.bat
```

### 手动构建

```bash
# 创建构建目录
mkdir build
cd build

# 配置 CMake（Windows）
cmake .. -G "Visual Studio 17 2022" -A x64

# 构建项目
cmake --build . --config Debug

# 可执行文件位置：build/Debug/test_client.exe
```

## 使用方法

### 1. 启动 Mini Server

首先确保 Mini Server 正在运行：

```bash
# 在 server 目录下
cd ../build/Debug
./MiniServer.exe
```

### 2. 运行测试客户端

```bash
# 基本用法（连接到 localhost:8080）
./test_client.exe

# 指定服务器地址
./test_client.exe localhost 8080

# 连接到其他服务器
./test_client.exe 192.168.1.100 9090
```

## 输出示例

```
=== Mini Server Test Client v1.0.0 ===

Starting comprehensive server tests...

Testing /ping endpoint...
✓ PASS: Ping test successful
  Response: {"status":"ok","message":"Server is running","timestamp":"..."}

Testing /services endpoint...
✓ PASS: Get services test successful
  Response: {"services":["echo","upper","reverse","length"]}

Testing /service/echo endpoint...
✓ PASS: Echo service test successful
  Input: Hello, World!
  Response: {"service":"echo","input":"Hello, World!","output":"Hello, World!"}

Testing /service/upper endpoint...
✓ PASS: Upper service test successful
  Input: hello world
  Response: {"service":"upper","input":"hello world","output":"HELLO WORLD"}

Testing /service/reverse endpoint...
✓ PASS: Reverse service test successful
  Input: 12345
  Response: {"service":"reverse","input":"12345","output":"54321"}

Testing /service/length endpoint...
✓ PASS: Length service test successful
  Input: test string
  Response: {"service":"length","input":"test string","length":11}

Testing non-existent service (should return 404)...
✓ PASS: Non-existent service correctly returned 404
  Response: {"error":"Service not found","service":"nonexistent"}

Testing invalid method on service endpoint...
✓ PASS: Invalid method correctly returned 405
  Response: {"error":"Method not allowed","method":"GET"}

=== Test Summary ===
Total tests: 8
Passed: 8
Failed: 0
Success rate: 100.0%

🎉 All tests passed! Server is working correctly.
```

## 代码结构

- `HttpClient` 类：底层 HTTP 客户端实现
- `TestClient` 类：测试逻辑和结果统计
- 完整的错误处理和异常捕获
- 详细的测试输出和结果统计

## 注意事项

1. 确保在运行测试客户端之前启动 Mini Server
2. 默认连接到 `localhost:8080`，可通过命令行参数修改
3. 测试客户端会自动处理网络初始化（Windows 下的 Winsock）
4. 所有测试都有超时保护，避免无限等待

## 故障排除

### 连接失败
- 检查 Mini Server 是否正在运行
- 验证主机名和端口号是否正确
- 检查防火墙设置

### 构建失败
- 确保安装了 CMake 3.15+
- 确保安装了支持 C++17 的编译器
- Windows 下确保安装了 Visual Studio 2019/2022

### 测试失败
- 检查服务器日志了解详细错误信息
- 确认服务器版本与测试客户端兼容
- 验证网络连接稳定性
