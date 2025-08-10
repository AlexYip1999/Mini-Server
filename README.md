# Mini Server - Modern C++17 HTTP Server

A professional-grade C++17 HTTP server with modular design and modern C++ best practices, supporting dynamic service registration and cross-platform deployment.

## 🌟 Core Features

- ⚡ **Modern C++17**: Latest C++ features, RAII, smart pointers, move semantics
- 🌐 **Cross-platform**: Full compatibility with Windows, Linux, macOS
- 🔒 **Thread Safe**: Multi-threaded concurrent processing with optimized read-write locks
- 📝 **Professional Logging**: Hierarchical logging system with color output and file recording
- 🎯 **Service Registration**: Dynamic service registration and management with hot-swapping
- 🛡️ **Error Handling**: Comprehensive exception safety and error recovery mechanisms
- 📚 **API Documentation**: Complete API documentation in Doxygen style
- ⚡ **High Performance**: Zero-dependency design with pure C++ standard library implementation

## 🏗️ Project Architecture

### Modular Design
```
miniserver::core      - Core server and routing logic
miniserver::http      - HTTP protocol handling and parsing
miniserver::network   - Cross-platform network abstraction layer
miniserver::services  - Dynamic service registration system
miniserver::utils     - Logging and utility modules
```

### 技术栈
- **C++17**: 现代C++标准，智能指针、移动语义、RAII
- **CMake 3.15+**: 跨平台构建系统
- **标准库**: 纯C++标准库，无第三方依赖
- **多线程**: std::thread、std::mutex、std::shared_mutex
- **网络**: 原生socket编程，Windows Winsock2 / Unix sockets

### 前端
- **HTML5/CSS3**: 现代Web标准
- **JavaScript**: 动态交互和数据更新
- **响应式设计**: 适配各种设备

## 快速开始

### 前置要求

- Visual Studio 2019/2022 或 MinGW-w64
- CMake 3.15+
- Windows 10/11

### 构建步骤

1. **克隆仓库**
   ```bash
   git clone <repository-url>
   cd mini-server
   ```

2. **安装依赖**
   
   下载cpp-httplib:
   ```bash
   # 方式1: 使用vcpkg
   vcpkg install cpp-httplib
   
   # 方式2: 手动下载
   # 从 https://github.com/yhirose/cpp-httplib 下载 httplib.h
   # 替换 third_party/httplib.h
   ```

3. **构建项目**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

4. **运行服务器**
   ```bash
   ./MiniServer.exe
   ```

5. **访问界面**
   
   打开浏览器访问: http://localhost:8080

### VS Code 构建

使用VS Code的任务系统：
- 按 `Ctrl+Shift+P`
- 选择 "Tasks: Run Task"
- 选择 "Build Project"

## 项目结构

```
mini-server/
├── CMakeLists.txt          # CMake配置文件
├── README.md              # 项目说明
├── include/               # 头文件目录
│   ├── server.h          # 服务器类
│   ├── system_monitor.h  # 系统监控类
│   └── web_handler.h     # Web处理类
├── src/                  # 源文件目录
│   ├── main.cpp         # 主入口
│   ├── server.cpp       # 服务器实现
│   ├── system_monitor.cpp # 系统监控实现
│   └── web_handler.cpp  # Web处理实现
├── third_party/         # 第三方库
│   └── httplib.h       # HTTP服务器库
└── web/                 # Web资源目录
    └── (静态文件)
```

## API接口

### 系统状态接口
- **GET** `/api/status` - 获取系统状态信息
- **GET** `/api/health` - 健康检查

### 响应示例

```json
{
  "cpu_usage": "45.2",
  "memory_usage": {
    "total": 17179869184,
    "used": 8589934592,
    "available": 8589934592,
    "percentage": 50.0
  },
  "disk_usage": {
    "total": 1000000000000,
    "used": 500000000000,
    "free": 500000000000,
    "percentage": 50.0
  },
  "network_info": {
    "interfaces": [
      {
        "name": "Ethernet",
        "status": "connected",
        "speed": "1Gbps"
      }
    ]
  },
  "uptime": 86400,
  "os_info": {
    "name": "Windows",
    "version": "10/11",
    "architecture": "x64"
  }
}
```

## 自定义配置

### 修改端口
在 `src/main.cpp` 中修改端口号：
```cpp
Server server(8080); // 改为你想要的端口
```

### 虚拟人物定制
在 `src/web_handler.cpp` 的HTML模板中：
- 修改 `messages` 数组来自定义对话内容
- 修改 `avatarExpressions` 数组来自定义表情
- 调整CSS样式来改变虚拟人物外观

## 扩展功能

### 添加新的监控指标
1. 在 `system_monitor.h` 中声明新方法
2. 在 `system_monitor.cpp` 中实现监控逻辑
3. 在 `get_system_info_json()` 中添加到JSON响应
4. 在前端HTML中添加显示元素

### 添加新的API端点
在 `server.cpp` 的 `run_server()` 方法中添加：
```cpp
server.Get("/api/new-endpoint", [](const httplib::Request&, httplib::Response& res) {
    res.set_content("your response", "application/json");
});
```

## 故障排除

### 常见问题

1. **编译错误 - 找不到 httplib.h**
   - 确保已正确下载并放置 cpp-httplib
   - 检查 CMakeLists.txt 中的包含路径

2. **服务器启动失败 - 端口被占用**
   - 检查端口8080是否被其他程序占用
   - 修改端口号或停止占用端口的程序

3. **无法获取系统信息**
   - 确保程序有足够的权限访问系统信息
   - 在Windows上可能需要管理员权限

## 性能优化

- 使用连接池减少资源开销
- 实现缓存机制避免频繁系统调用
- 使用WebSocket替代轮询获得更好的实时性

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request！

## 联系方式

如有问题，请通过GitHub Issues联系。
