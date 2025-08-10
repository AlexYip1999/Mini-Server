# Mini Server - C++17 HTTP服务器

## 项目结构

```
mini-server/
├── .github/                    # GitHub配置
├── .vscode/                    # VS Code配置
├── build/                      # 构建输出目录 (CMake生成)
├── miniserver/                 # 头文件 (现代C++17架构)
│   ├── core/                   # 核心模块
│   │   ├── server.hpp         # 主服务器类
│   │   └── request_router.hpp # 请求路由器
│   ├── http/                   # HTTP协议处理
│   │   ├── http_types.hpp     # HTTP类型定义
│   │   └── http_parser.hpp    # HTTP解析器
│   ├── network/                # 网络抽象层
│   │   └── socket_server.hpp  # 跨平台套接字服务器
│   ├── services/               # 服务注册系统
│   │   └── service_registry.hpp # 服务注册表
│   └── utils/                  # 工具模块
│       └── logger.hpp         # 日志系统
├── source/                     # 源文件实现
│   ├── core/                   # 核心模块实现
│   ├── http/                   # HTTP协议实现
│   ├── network/                # 网络层实现
│   ├── services/               # 服务系统实现
│   ├── utils/                  # 工具模块实现
│   └── main.cpp               # 应用程序入口
├── src/                        # 旧版实现 (将被移除)
├── CMakeLists.txt             # CMake构建配置
└── README.md                  # 项目说明
```

## 主要特性

- 🔧 **现代C++17**：使用最新C++特性和最佳实践
- 🌐 **跨平台**：支持Windows、Linux、macOS
- 🔒 **线程安全**：多线程并发处理，使用读写锁优化性能
- 📝 **完整日志**：分级日志系统，支持控制台和文件输出
- 🎯 **服务注册**：动态服务注册和管理系统
- 🛡️ **错误处理**：完善的异常处理和错误报告
- 📚 **详细文档**：Doxygen风格的API文档

## 构建说明

```bash
# 配置项目
cmake -B build -S .

# 编译项目
cmake --build build --config Release

# 运行服务器
./build/Release/MiniServer.exe --port 8080
```

## API端点

- `GET /ping` - 健康检查
- `POST /service/{name}` - 调用注册的服务
- `GET /services` - 获取所有服务列表
- `OPTIONS *` - CORS支持

## 架构优势

本项目采用现代C++17设计模式：

1. **模块化设计**：每个功能模块独立，便于维护和测试
2. **命名空间组织**：`miniserver::core`、`::http`、`::network`等
3. **RAII原则**：资源自动管理，避免内存泄漏
4. **异常安全**：完善的异常处理机制
5. **可扩展性**：易于添加新功能和服务

该架构适合作为微服务基础框架或学习现代C++的参考项目。
