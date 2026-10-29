# LinkChat 命名速查表（Google C++ Style）

写新代码时对照本表。本项目仅统一了**命名**，缩进仍为 4 空格（非 Google 的 2 空格/80 列）。

## 一、速查表

| 对象 | 规则 | 本项目示例 |
|---|---|---|
| 文件 | 全小写 + 下划线；头 `.h`，实现 `.cc` | `elastic_client.h`、`mq_test.cc` |
| 类型 / 类 / struct / 枚举 / using 别名 | `CamelCase`；缩写当单词（不用全大写） | `OssClient`、`MqClient`、`JsonCodec`、`Consumer::Ptr` |
| 函数 | `CamelCase` | `GetExchangeType`、`WaitForConnection` |
| 变量（含参数、局部） | `snake_case`，无前缀 | `routing_key`、`delivery_tag` |
| 类成员变量 | `snake_case_`（尾下划线） | `declare_mutex_`、`etcd_server_ip_` |
| **struct 成员**（纯数据） | `snake_case`，**不带**尾下划线 | `DeclareSettings{exchange, queue, ttl}`、`MailMessage{from, to, ...}` |
| 常量 | `k` + `CamelCase`，`inline constexpr` | `kMaxRetry`、`kDlxPrefix`、`kRpcProtocol` |
| 全局变量 | `g_` + `snake_case`，`inline` | `g_logger`、`g_svc_channels` |
| 宏 | 全大写，带语义前缀 | `LOG_TRACE/DEBUG/INFO/WARN/ERROR/CRITICAL` |
| 命名空间 | 全小写，可为 `snake_case` | `exchange_type`、`dead_letter`、`rpc_constants`、`etcd_constants` |
| 纯数据 struct | 聚合初始化，不写构造函数 | `DeclareSettings{"exchange", exchange_type::kDirect, "queue", "key", 9}` |
| 测试文件 | `<被测对象>_test.cc` | `rpc_test.cc` |
| 测试用例 | `TEST(Suite, Case)` 名字里**不能有下划线** | `TEST(RpcWrapper, AddNodeAndGetChannel)` |

## 二、写代码时必须遵守的几条

1. **头文件里定义函数一律加 `inline`**（自由函数、类外定义的成员函数都要加），否则多个 `.cc` 同时 include 会 `multiple definition`。类内就地定义的函数、`static` 成员函数自带 inline 语义，无需再加。
   - 头文件里的全局对象也必须 `inline`（如 `inline std::shared_ptr<spdlog::logger> g_logger;`）。
   - 实现较大的模块请拆成 `.h`（声明）+ `.cc`（定义），这是 Google 的标准做法。
2. **常量写成 `inline constexpr`，并放进 namespace**，不要用 `static const std::string`，也不要用 `class XxxConstants`：
   ```cpp
   namespace exchange_type {
       inline constexpr char kFanout[] = "fanout";
   }
   ```
3. **using 别名用 `CamelCase`**：`using Ptr = std::shared_ptr<Foo>;`，不要写 `using ptr`。
4. **缩写当单词**：`MqClient`、`OssClient`、`StartRpc()`，不要 `MQClient` / `StartRPC()`（第三方库符号除外）。
5. **头文件保护统一用 `#pragma once`**，不要与 `#ifndef` 混用。
6. **变参宏用标准 `__VA_OPT__`**（需 C++20），不要用 GCC 扩展 `##__VA_ARGS__`。

## 三、不能按 C++ 规范改名的"契约名字"

跨进程 / 跨存储 / 外部依赖的名字，改名会破坏兼容性：

| 类别 | 例子 | 说明 |
|---|---|---|
| proto 字段名 | `req_message`、`resp_message` | 影响 JSON/wire 兼容。字段永远 `snake_case`；消息名 `CamelCase`（`EchoRequest`）；枚举值 `UPPER_SNAKE` |
| 数据库表/列 | `tb_person`、`id`/`name`/`age` | 由 ODB 从成员名派生（**自动去掉尾下划线**）。改成员名后要确认重新生成的 `person.sql` 未变，必要时用 `#pragma db column("...")` 锁死 |
| gflags 参数名 | `DEFINE_string(smtp_url, ...)` | 是命令行 / `settings.conf` 契约，必须 `snake_case`；代码里用 `FLAGS_smtp_url` |
| AMQP 交换机/队列/路由键/死信 header | `test_exchange`、`x-dead-letter-exchange` | Broker 侧契约 |
| CMake target 名 | `oss_test`、`amqp_pub` | 外部脚本 / IDE 依赖 |