<p align="center">
  <img src="logo.png" width="160" alt="LinkChat logo">
</p>

<h1 align="center">LinkChat</h1>

<p align="center">一个基于 C++ 的微服务开发脚手架</p>

## 简介

本仓库是一套可直接上手的 C++ 微服务脚手架：

- **开箱即用的中间件环境**：一份 `docker-compose.yml` 拉起 MySQL、Redis、RabbitMQ、Elasticsearch、Kibana、Etcd 以及 C++ 开发容器。
- **常用中间件的客户端封装**：`server/src/adapter/` 下按「一个中间件一个适配器」的方式封装，包含 RPC、服务注册发现、消息队列、缓存、搜索、ORM、对象存储、邮件、日志、序列化。
- **每个中间件的最小可运行示例**：`examples/` 下按中间件分目录，每个目录是一个可独立编译的最小工程，方便对照学习或摘抄。

## 技术栈

| 分类 | 选型 |
|---|---|
| 语言 / 构建 | C++20、CMake 3.22+ |
| RPC | brpc + protobuf |
| 服务注册与发现 | etcd（`etcd-cpp-api`） |
| 消息队列 | RabbitMQ（`AMQP-CPP` + libev） |
| 缓存 | Redis（`redis++`） |
| 搜索 | Elasticsearch（`elasticlient` + `cpr`） |
| 数据库 / ORM | MySQL + ODB |
| 对象存储 | 阿里云 OSS C++ SDK |
| HTTP / WebSocket | `cpp-httplib`、`websocketpp` |
| 日志 | spdlog（异步 + 彩色输出） |
| 命令行参数 | gflags |
| 序列化 | jsoncpp、protobuf |
| 测试 | gtest |

## 目录结构

```
.
├── server/src/                 # 框架源码
│   ├── adapter/                # 中间件适配器
│   │   ├── amqp/               #   MQ 客户端封装、Publisher / Consumer
│   │   ├── elastic/            #   ES 客户端封装
│   │   ├── etcd/discover/      #   服务发现
│   │   ├── etcd/registry/      #   服务注册与续租
│   │   ├── json/               #   JSON 编解码
│   │   ├── logging/            #   日志器与 LOG_* 宏
│   │   ├── mail/               #   邮件发送（libcurl）
│   │   ├── oss/                #   阿里云 OSS 上传 / 预签名下载
│   │   ├── redis/              #   Redis 连接工厂
│   │   └── rpc/                #   brpc 信道管理、回调封装
│   ├── constants/              # 常量（AMQP、etcd、brpc）
│   ├── util/                   # 文件、字符串、随机 ID 工具
│   └── test/                   # 各适配器的测试/演示程序 + CMakeLists.txt
├── examples/                   # 各中间件的最小示例
├── deploy/                     # docker-compose 与各中间件配置文件
├── image_build/                # 开发镜像 Dockerfile
├── settings.conf               # 本地配置（含密钥，已被 .gitignore 忽略）
├── template.conf               # 配置模板，复制为 settings.conf 后填写
├── NAMING.md                   # 命名规范速查表（Google C++ Style）
└── logo.png
```

## 快速开始

### 1. 启动依赖服务

```bash
cd deploy
docker compose up -d
```

### 2. 进入开发容器

开发容器映射了 SSH 端口 `2222` 与业务端口 `9000-9004`，源码目录挂载在容器的 `/home/dev/workspace`：

```bash
ssh -p 2222 <user>@<host>
```

### 3. 准备配置

```bash
cp template.conf settings.conf   # 然后填入 SMTP 账号、阿里云 AK/SK 等
```

`settings.conf` 里是明文密钥，已被 `.gitignore` 忽略，**不要提交**。

### 4. 编译并运行

以 `server/src/test` 为例（各示例目录同理）：

```bash
cmake -S server/src/test -B build
cmake --build build -j

# 需要读取 settings.conf 的程序用 flagfile 传入
cd build && ./oss_test --flagfile=../settings.conf
```

> 测试目标默认只开启 `oss_test`，其余目标在 `server/src/test/CMakeLists.txt` 的 `set(targets ...)` 里按需打开。

## 服务与端口

| 服务 | 容器名 | 端口 |
|---|---|---|
| 开发环境（SSH / 业务） | `cpp-scaffold` | `2222` / `9000-9004` |
| MySQL | `mysql-service` | `3306` |
| Redis | `redis-service` | `6379` |
| RabbitMQ | `rabbitmq-service` | `5672`（AMQP）、`15672`（控制台） |
| Elasticsearch | `elasticsearch-service` | `9200`、`9300` |
| Kibana | `kibana-service` | `5601` |
| Etcd | `etcd-service` | `2379` |

容器内访问这些服务时直接用服务名，例如 `http://etcd-service:2379`、`amqp://admin:123456@rabbitmq-service:5672/`。

## 示例一览

| 目录 | 内容 |
|---|---|
| `examples/amqp` | 发布 / 订阅消息 |
| `examples/brpc` | protobuf + brpc 同步与异步调用 |
| `examples/elastic` | 索引创建、写入、查询、删除 |
| `examples/etcd` | 键值读写、租约保活、监听变更 |
| `examples/gflags` | 命令行参数与配置文件 |
| `examples/gtest` | 全局事件、测试套件、断言 |
| `examples/httplib` | HTTP 服务与静态资源托管 |
| `examples/jsoncpp` | JSON 序列化 / 反序列化 |
| `examples/odb` | ODB + MySQL 的增删改查与事务 |
| `examples/proto` | protobuf 与 JSON 互转 |
| `examples/redis` | string / list / hash / set 操作 |
| `examples/spdlog` | 多落地方向与异步日志 |
| `examples/websocketpp` | WebSocket / HTTP 服务 |

## 命名规范

代码统一遵循 Google C++ Style 的命名约定，速查表见 [NAMING.md](NAMING.md)。提交新代码前请对照检查，尤其是头文件内函数需加 `inline`、常量用 `inline constexpr` 两点。

## 注意

- 除 `logo.png`、`settings.conf`、`deploy/`、`image_build/` 外的源码目录均为手写代码；第三方库头文件（`.hpp`）保持原样，不在命名规范约束范围内。
- `build/` 是构建产物目录。改名或替换 proto 后请重新生成，否则旧的生成文件（如 `echo.pb.h`）可能误导排查。
