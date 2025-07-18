# ChatServer

ChatServer 是一个基于 C++ 开发的高性能分布式聊天服务器，支持私聊、群聊、离线消息等功能。该项目采用 muduo 网络库实现高并发通信，并使用 Redis 发布-订阅模式实现跨服务器的消息转发，构建了一个可扩展的分布式聊天系统。

## 功能特性

- **用户管理**：注册、登录、注销、状态显示
- **好友系统**：添加好友、查看好友列表、好友私聊
- **群组系统**：创建群组、加入群组、群聊
- **离线消息**：自动存储离线消息，用户上线时推送
- **分布式架构**：支持多服务器部署，实现跨服务器消息转发
- **高并发处理**：基于 muduo 网络库的事件驱动模型，高效处理并发连接

## 技术栈

- **编程语言**：C++11
- **网络库**：muduo
- **数据库**：MySQL
- **缓存与消息队列**：Redis
- **JSON解析**：nlohmann/json
- **构建工具**：CMake
- **线程池**：自定义线程池实现

## 系统架构

```
                  ┌─────────────┐
                  │   Client    │
                  └─────────────┘
                         │
                         ▼
┌─────────────────────────────────────────┐
│              ChatServer                 │
├─────────────┬─────────────┬─────────────┤
│ ChatService │ UserModel   │ FriendModel │
├─────────────┼─────────────┼─────────────┤
│ GroupModel  │ OfflineMsg  │ Redis       │
└─────────────┴─────────────┴─────────────┘
         │                        │
         ▼                        ▼
┌─────────────────┐      ┌─────────────────┐
│     MySQL       │      │     Redis       │
└─────────────────┘      └─────────────────┘
```

## 安装指南

### 前提条件

- Linux 操作系统
- GCC 7.1+ 编译器（支持 C++11）
- CMake 3.0+
- MySQL 5.7+
- Redis 5.0+
- muduo 网络库

### 安装步骤

1. 克隆仓库

```bash
git clone https://github.com/yourusername/ChatServer.git
cd ChatServer
```

2. 创建并初始化数据库

```bash
# 登录 MySQL
mysql -u root -p

# 在 MySQL 中执行以下命令
CREATE DATABASE chat;
USE chat;

# 创建用户表
CREATE TABLE User (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(50) NOT NULL,
    state ENUM('online', 'offline') DEFAULT 'offline'
);

# 创建好友表
CREATE TABLE Friend (
    userid INT NOT NULL,
    friendid INT NOT NULL,
    PRIMARY KEY (userid, friendid)
);

# 创建群组表
CREATE TABLE AllGroup (
    id INT PRIMARY KEY AUTO_INCREMENT,
    groupname VARCHAR(50) NOT NULL UNIQUE,
    groupdesc VARCHAR(200) DEFAULT ''
);

# 创建群组成员表
CREATE TABLE GroupUser (
    groupid INT NOT NULL,
    userid INT NOT NULL,
    role ENUM('creator', 'normal') DEFAULT 'normal',
    PRIMARY KEY (groupid, userid)
);

# 创建离线消息表
CREATE TABLE OfflineMessage (
    userid INT NOT NULL,
    message VARCHAR(500) NOT NULL
);
```

3. 编译项目

```bash
mkdir build && cd build
cmake ..
make
```

## 配置说明

在运行服务器前，请确保配置文件 `config.json` 已正确设置：

```json
{
    "server": {
        "ip": "127.0.0.1",
        "port": 6000
    },
    "mysql": {
        "host": "127.0.0.1",
        "port": 3306,
        "username": "root",
        "password": "your_password",
        "dbname": "chat"
    },
    "redis": {
        "host": "127.0.0.1",
        "port": 6379
    }
}
```

## 使用方法

### 启动服务器

```bash
cd build
./ChatServer 127.0.0.1 6000
```

### 客户端命令

客户端可以发送 JSON 格式的消息与服务器交互，主要命令如下：

#### 用户注册

```json
{
    "msgid": 1,
    "name": "username",
    "password": "password"
}
```

#### 用户登录

```json
{
    "msgid": 2,
    "id": 1,
    "password": "password"
}
```

#### 添加好友

```json
{
    "msgid": 3,
    "id": 1,
    "friendid": 2
}
```

#### 私聊消息

```json
{
    "msgid": 4,
    "id": 1,
    "from": "username1",
    "to": "username2",
    "msg": "hello"
}
```

#### 创建群组

```json
{
    "msgid": 5,
    "id": 1,
    "groupname": "group1",
    "groupdesc": "This is group1"
}
```

#### 加入群组

```json
{
    "msgid": 6,
    "id": 1,
    "groupid": 1
}
```

#### 群聊消息

```json
{
    "msgid": 7,
    "id": 1,
    "groupid": 1,
    "from": "username1",
    "msg": "hello everyone"
}
```

#### 注销

```json
{
    "msgid": 8,
    "id": 1
}
```

## 项目结构

```
ChatServer/
├── include/                # 头文件
│   ├── server/            # 服务器相关头文件
│   │   ├── chatserver.hpp # 聊天服务器类
│   │   ├── chatservice.hpp # 业务逻辑类
│   │   ├── model/         # 数据模型
│   │   │   ├── user.hpp
│   │   │   ├── friend.hpp
│   │   │   ├── group.hpp
│   │   │   ├── groupuser.hpp
│   │   │   └── offlinemessagemodel.hpp
│   │   └── redis/         # Redis 相关
│   │       └── redis.hpp
│   └── protocol.hpp       # 通信协议定义
├── src/                   # 源文件
│   ├── server/            # 服务器实现
│   │   ├── chatserver.cpp
│   │   ├── chatservice.cpp
│   │   ├── model/         # 数据模型实现
│   │   │   ├── usermodel.cpp
│   │   │   ├── friendmodel.cpp
│   │   │   ├── groupmodel.cpp
│   │   │   └── offlinemessagemodel.cpp
│   │   └── redis/         # Redis 实现
│   │       └── redis.cpp
│   └── main.cpp           # 主函数
├── CMakeLists.txt         # CMake 构建文件
├── config.json            # 配置文件
└── README.md              # 项目说明文档
```

## 性能与扩展性

- 服务器采用事件驱动模型，能够高效处理大量并发连接
- 通过 Redis 发布-订阅模式实现分布式架构，支持水平扩展
- 数据库操作封装为模型类，便于维护和扩展

## 许可证

本项目采用 MIT 许可证，详情请参阅 [LICENSE](LICENSE) 文件。

## 贡献指南

欢迎提交 Issue 和 Pull Request 来帮助改进项目。在提交 PR 前，请确保代码符合项目的编码规范，并通过所有测试。

## 联系方式

如有任何问题或建议，请通过 GitHub Issues 与我们联系。