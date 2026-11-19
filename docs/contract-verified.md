# AGENTS.md §6.5 契约与真实后端差异清单

> 对比基准：`src/server/` 全部 proto + gateway 路由 + 真实 HTTP 响应实测
> 生成日期：2026-11-19

---

## 0. 全局约定（§6.2 修正）

| 项 | AGENTS.md 声明 | 真实后端 | 影响 |
|---|---|---|---|
| URL 前缀 | `path` 以 `/` 开头，不含 `/api` | ✅ **一致**。所有 gateway 路由都是 `/xxx/xxx`，没有 `/api` 前缀 | 无 |
| 鉴权头 | `Authorization: <token>`（不加 `Bearer`） | ✅ **一致**，见 gateway.h L100 | 无 |
| 基地址 | `VITE_API_BASE=http://127.0.0.1:9000` | ✅ gateway 跑在 9000 | 无 |
| 响应包装 | 未声明 | **所有响应统一 `{ success: bool, errmsg: string, ...业务字段 }`**，但 proto3 JSON 默认值字段会被省略：`bool=false`、`string=""` 不出现在 JSON 里 | ⚠️ **高优先级**：前端判断成功不能假设 `success` 存在 |

### ⚠️ 响应 `success` 字段缺失陷阱

真实验证（POST `/login/nickname` 返回 `{"errmsg":"用户不存在"}`）确认：

- 后端用 proto3 `MessageToJsonString` 序列化，**默认值字段会被省略**
- `bool success = false` → JSON 中 **无** `success` 字段
- `string errmsg = ""` → JSON 中 **无** `errmsg` 字段
- 只有 `success = true` 时才会输出 `"success": true`

**前端正确判断**：`resp.success === true` 才算成功，缺失 `success` 字段 = 失败。

### ⚠️ GET 参数 vs POST Body 命名风格不一致

Gateway 对 GET 请求用 `req.get_param_value("snake_case")` 直接读取 query string，对 POST 请求体用 proto3 JSON 解析。

| 传输方式 | 字段命名 | 示例 |
|---|---|---|
| **GET 请求参数**（query string） | **snake_case** | `chat_session_id`, `start_time`, `msg_count`, `file_id` |
| **POST 请求体**（JSON） | lowerCamelCase 和 snake_case **都能解析**（proto3 JSON 兼容） | 建议前端统一用 lowerCamelCase |
| **所有响应**（JSON） | **lowerCamelCase**（proto3 JSON 默认） | `chatSessionId`, `startTime`, `msgCount`, `fileId` |

**前端必须遵守**：GET 请求 URL 参数用 `snake_case` 拼接。

---

## 1. user.ts — 登录 & 用户信息

### 契约声明 vs 真实后端

| AGENTS.md §6.5 声明 | 真实后端 | 差异等级 |
|---|---|---|
| `login({ username, password }): Promise<{ token }>` | **不存在此函数！** 有两个登录接口：<br>`POST /login/nickname` body:`{ nickname, password }`<br>`POST /login/email` body:`{ email, verifyCode }` | 🔴 **函数不存在** |
| `getProfile(): Promise<UserInfo>` | `GET /user/info`，**无参数**（user_id 从 token 解析）→ `{ success, errmsg, userInfo }` | 🟡 路径和函数名没问题，但无参数 |

### 完整 user 端点（真实）

| 方法 | 路径 | 白名单 | 请求 | 响应（lowerCamelCase） |
|---|---|---|---|---|
| POST | `/register` | ✅ | body: `{ nickname, password, email, verifyCode }` | `{ success?, errmsg? }` |
| GET | `/login/mail_verify_code` | ✅ | query: `email=xxx` | `{ success?, errmsg? }` |
| POST | `/login/nickname` | ✅ | body: `{ nickname, password }` | `{ success?, errmsg?, token? }` |
| POST | `/login/email` | ✅ | body: `{ email, verifyCode }` | `{ success?, errmsg?, token? }` |
| GET | `/user/info` | ❌ | 无参数 | `{ success?, errmsg?, userInfo: UserInfo }` |
| GET | `/user/multi_info` | ❌ | query: `users_id=xxx&users_id=yyy...` | `{ success?, errmsg?, usersInfo: Map<string, UserInfo> }` |
| POST | `/user/avatar/set` | ❌ | multipart/form-data: `avatar` 文件 | `{ success?, errmsg? }` |
| POST | `/user/nickname/set` | ❌ | query: `nickname=xxx` | `{ success?, errmsg? }` |
| POST | `/user/description/set` | ❌ | query: `description=xxx` | `{ success?, errmsg? }` |

### UserInfo 字段（真实 proto）

| 字段 | 类型 | 说明 |
|---|---|---|
| `userId` | string | 用户ID |
| `nickname` | string | 昵称 |
| `description` | string | 个人签名 |
| `email` | string | 邮箱 |
| `avatar` | string | 头像预签名 URL |

### 修正建议

```ts
// 替换 login 为两个独立登录
export function nicknameLogin(p: { nickname: string; password: string })
  : Promise<{ success?: boolean; errmsg?: string; token?: string }>
export function emailLogin(p: { email: string; verifyCode: string })
  : Promise<{ success?: boolean; errmsg?: string; token?: string }>
export function register(p: { nickname: string; password: string; email: string; verifyCode: string })
  : Promise<{ success?: boolean; errmsg?: string }>
export function getEmailVerifyCode(email: string)
  : Promise<{ success?: boolean; errmsg?: string }>
export function getProfile()
  : Promise<{ success?: boolean; errmsg?: string; userInfo: UserInfo }>
```

---

## 2. message.ts — 消息历史 / 最近 / 搜索 / 发送

### 契约声明 vs 真实后端

| AGENTS.md §6.5 声明 | 真实后端 | 差异等级 |
|---|---|---|
| `getHistory({ sessionId, startTime, endTime })` | `GET /message/history` 参数名全错：<br>`chat_session_id`（不是 sessionId）<br>`start_time`（不是 startTime，单位分钟）<br>`over_time`（不是 endTime，单位分钟）<br>必填三个参数 | 🔴 **参数名全错** |
| `getRecent({ sessionId, curTime? })` | `GET /message/recent` **缺少必填参数 `msg_count`**：<br>`chat_session_id`（必填）<br>`msg_count`（必填，整数）<br>`cur_time`（可选，整数，缺省服务端当前时间） | 🔴 **漏了 msg_count 必参** |
| `searchMessage({ sessionId, keyword })` | `GET /message/search` 参数名不同：<br>`chat_session_id`（不是 sessionId）<br>`search_key`（不是 keyword） | 🟡 参数名不同 |
| `publishMessage(body): Promise<void>` | `POST /transmite/publish` **有返回值**：`{ success?, errmsg? }`<br>请求体：`{ chatSessionId, message: { messageType, stringMessage|fileMessage } }` | 🟡 不是 void |

### 完整 message 端点（真实）

| 方法 | 路径 | 请求参数（query=snake） | 响应 |
|---|---|---|---|
| GET | `/message/history` | `chat_session_id`、`start_time`（分钟）、`over_time`（分钟） | `{ success?, errmsg?, msgList: MessageInfo[] }` |
| GET | `/message/recent` | `chat_session_id`、`msg_count`（整数）、`cur_time`（可选，分钟） | `{ success?, errmsg?, msgList: MessageInfo[] }` |
| GET | `/message/search` | `chat_session_id`、`search_key` | `{ success?, errmsg?, msgList: MessageInfo[] }` |
| POST | `/transmite/publish` | body JSON: `{ chatSessionId, message: MessageContent }` | `{ success?, errmsg? }` |

### ⚠️ 时间单位关键区别

| 来源 | 字段 | 单位 | 类型 |
|---|---|---|---|
| **HTTP 请求参数** | `start_time`、`over_time`、`cur_time` | **Unix 分钟** | int64 |
| **消息结构内** | `createTime` | **Unix 秒**（proto3 int64 → JSON string） | string |

前端构造查询参数时必须做 `Math.floor(unixSeconds / 60)` 转换成分钟。

### MessageInfo 完整结构（真实 proto + proto3 JSON）

```typescript
interface MessageInfo {
  messageId: string;           // 消息ID
  chatSessionId: string;       // 所属会话ID
  sender: UserInfo;            // 发送者完整信息
  message: MessageContent;     // 消息体
  createTime: string;          // Unix 秒（int64 → JSON string）
}

interface MessageContent {
  messageType: 'STRING' | 'FILE';  // 枚举，不是 number
  stringMessage?: StringMessageInfo;
  fileMessage?: FileMessageInfo;
}

interface StringMessageInfo {
  content: string;
}

interface FileMessageInfo {
  fileId?: string;
  fileSize?: string;           // int64 → JSON string
  fileName?: string;
  fileContent?: string;        // base64 bytes → JSON string
}

// ChatSessionInfo（会话列表用）
interface ChatSessionInfo {
  singleChatFriendId?: string;
  chatSessionId: string;
  chatSessionName: string;
  prevMessage?: MessageInfo;
  avatar: string;
}
```

### 修正建议

```ts
// 历史消息 — 参数名、时间单位全对
export function getHistory(p: {
  chatSessionId: string;
  startTime: number;  // Unix 秒，内部 /60 转分钟拼 URL
  overTime: number;   // Unix 秒，内部 /60 转分钟拼 URL
}): Promise<{ success?: boolean; errmsg?: string; msgList: MessageInfo[] }>

// 最近 N 条 — 补 msgCount 必参
export function getRecent(p: {
  chatSessionId: string;
  msgCount: number;       // ✅ 新增必填
  curTime?: number;       // Unix 秒，可选
}): Promise<{ success?: boolean; errmsg?: string; msgList: MessageInfo[] }>

// 搜索消息 — keyword → searchKey，sessionId → chatSessionId
export function searchMessage(p: {
  chatSessionId: string;  // 原 sessionId
  searchKey: string;      // 原 keyword
}): Promise<{ success?: boolean; errmsg?: string; msgList: MessageInfo[] }>

// 发送消息 — 有返回值不是 void
export interface PublishReq {
  chatSessionId: string;
  message: {
    messageType: 'STRING' | 'FILE';
    stringMessage?: { content: string };
    fileMessage?: { fileSize: string; fileName: string; fileContent: string };
  };
}
export function publishMessage(body: PublishReq): Promise<{ success?: boolean; errmsg?: string }>
```

---

## 3. friend.ts — 好友 & 会话

### 契约声明 vs 真实后端

| AGENTS.md §6.5 声明 | 真实后端 | 差异等级 |
|---|---|---|
| `getFriendList(): Promise<FriendInfo[]>` | `GET /friend/list` → `{ friendList: UserInfo[] }`<br>**无 FriendInfo 类型**，返回的就是 `UserInfo[]` | 🟡 返回类型不同 |
| `getApplyList(): Promise<ApplyInfo[]>` | 真实路径是 `GET /friend/pending_event`<br>返回 `{ event: FriendEvent[] }`<br>**函数名和路径全错** | 🔴 **函数名+路径都错** |

### 完整 friend 端点（真实，远超 AGENTS.md 声明）

| 方法 | 路径 | 请求 | 响应 |
|---|---|---|---|
| GET | `/friend/list` | 无参数（user_id 从 token） | `{ success?, errmsg?, friendList: UserInfo[] }` |
| POST | `/friend/remove` | body: `{ peerId }` | `{ success?, errmsg? }` |
| POST | `/friend/add` | body: `{ respondentId }` | `{ success?, errmsg?, notifyEventId? }` |
| POST | `/friend/add_process` | body: `{ notifyEventId, agree, applyUserId }` | `{ success?, errmsg?, newSessionId? }` |
| GET | `/friend/search` | query: `search_key=X&count=N` | `{ success?, errmsg?, userInfo: UserInfo[] }` |
| GET | `/friend/pending_event` | 无参数 | `{ success?, errmsg?, event: FriendEvent[] }` |
| GET | `/friend/chat_session/list` | 无参数 | `{ success?, errmsg?, chatSessionInfoList: ChatSessionInfo[] }` |
| POST | `/friend/chat_session/create` | body: `{ chatSessionName, memberIdList }` | `{ success?, errmsg?, chatSessionId? }` |
| GET | `/friend/chat_session/member` | query: `chat_session_id=X` | `{ success?, errmsg?, memberInfoList: UserInfo[] }` |

### FriendEvent 结构

```typescript
interface FriendEvent {
  eventId: string;
  sender: UserInfo;
}
```

### 修正建议

```ts
// ✅ 函数名正确，返回 UserInfo 不是 FriendInfo
export function getFriendList(): Promise<{ success?: boolean; errmsg?: string; friendList: UserInfo[] }>

// ✅ 原 getApplyList 路径+函数名都错
export function getPendingFriendEvents(): Promise<{
  success?: boolean; errmsg?: string; event: FriendEvent[];
}>

// ✅ 以下是 AGENTS.md 未声明但后端已有的接口
export function removeFriend(peerId: string): Promise<{ success?: boolean; errmsg?: string }>
export function addFriend(respondentId: string): Promise<{ success?: boolean; errmsg?: string; notifyEventId?: string }>
export function processFriendApply(p: { notifyEventId: string; agree: boolean; applyUserId: string })
  : Promise<{ success?: boolean; errmsg?: string; newSessionId?: string }>
export function searchFriend(p: { searchKey: string; count: number })
  : Promise<{ success?: boolean; errmsg?: string; userInfo: UserInfo[] }>
export function getChatSessionList()
  : Promise<{ success?: boolean; errmsg?: string; chatSessionInfoList: ChatSessionInfo[] }>
export function createChatSession(p: { chatSessionName: string; memberIdList: string[] })
  : Promise<{ success?: boolean; errmsg?: string; chatSessionId?: string }>
export function getChatSessionMember(chatSessionId: string)
  : Promise<{ success?: boolean; errmsg?: string; memberInfoList: UserInfo[] }>
```

---

## 4. file.ts — 文件下载（后端有，AGENTS.md 没声明）

### 真实端点

| 方法 | 路径 | 请求 | 响应 |
|---|---|---|---|
| GET | `/file/get_single` | query: `file_id=xxx` | `{ success?, errmsg?, fileData?: FileDownloadData }` |
| GET | `/file/get_multi` | query: `file_ids=a&file_ids=b...` | `{ success?, errmsg?, fileData: Map<string, FileDownloadData> }` |

### FileDownloadData

```typescript
interface FileDownloadData {
  fileId: string;
  presignedUrl: string;  // 预签名下载地址
}
```

**注**：后端真实暴露的文件接口只有下载，上传由 `transmite/publish` 的 `fileMessage` 内联 base64 完成（见 §2）。

---

## 5. WebSocket 推送（AGENTS.md §6.7 对照）

| 推送 type | AGENTS.md 声明 data | 后端 gateway 代码确认 | 结论 |
|---|---|---|---|
| `NEW_MESSAGE` | 完整 MessageInfo | ✅ gateway.h 序列化 `GetTransmitTargetRsp.message` 推送 | **一致** |
| `FRIEND_REQUEST` | `{ notifyEventId, applyUserId }` | ✅ gateway.h L1349-1352 确认 `data["notifyEventId"]` + `data["applyUserId"]` | **一致** |
| `FRIEND_APPLY_RESULT` | `{ agree, newSessionId? }` | ⚠️ **gateway.h 中未找到推送代码**，`FriendAddProcess` 只返回 `newSessionId`，推送可能在 friend_service 内部 | **待验证** |
| `CHAT_SESSION_CREATED` | `{ chatSessionId, chatSessionName, creatorId }` | ⚠️ **gateway.h 中未找到推送代码** | **待验证** |

**信封结构** `{"type": "...", "data": {...}}` — ✅ gateway.h `BuildPushPayload` 确认一致。

---

## 6. 差异汇总（按优先级排序）

### 🔴 高优先级（不改无法联调）

| # | 位置 | 问题 | 修正 |
|---|---|---|---|
| 1 | user.ts | `login({username,password})` 函数**不存在** | 拆成 `nicknameLogin` + `emailLogin` |
| 2 | message.ts getHistory | 参数名全错（`sessionId`→`chat_session_id`，`startTime`→`start_time`，`endTime`→`over_time`），时间单位是分钟 | 改参数名 + 加分钟转换 |
| 3 | message.ts getRecent | **缺少必填 `msg_count` 参数** | 补 `msgCount: number` |
| 4 | message.ts searchMessage | 参数名错：`keyword`→`search_key`，`sessionId`→`chat_session_id` | 改参数名 |
| 5 | friend.ts | `getApplyList()` 函数名+路径**全错**，真实是 `getPendingFriendEvents()` → `GET /friend/pending_event` | 重写 |
| 6 | 全局 | **GET 请求参数用 snake_case**，不是 lowerCamelCase | URL 拼接时用 snake_case |
| 7 | 全局 | **响应 `success` 字段会缺失**（proto3 默认值省略） | 前端判断用 `resp.success === true` |

### 🟡 中优先级（有影响但不阻塞骨架）

| # | 位置 | 问题 | 修正 |
|---|---|---|---|
| 8 | user.ts | `getProfile()` 路径 `/user/info` 正确但返回的是 `{ userInfo }` 不是直接 `UserInfo` | 改类型 |
| 9 | friend.ts | `getFriendList()` 返回 `UserInfo[]` 不是 `FriendInfo[]` | 改类型 |
| 10 | message.ts | `publishMessage()` 返回 `{ success, errmsg }` 不是 `void` | 改返回类型 |
| 11 | message.ts | 所有接口响应都是 `{ success, errmsg, msgList }` 包裹，不是直接 `MessageInfo[]` | 改返回类型 |
| 12 | user.ts | 响应 token 路径是 `resp.token` 不是 `resp.data.token`（没有 data 层） | 确认 |

### 🟢 低优先级（后端有但 AGENTS.md 未声明的接口）

| # | 接口 | 说明 |
|---|---|---|
| A | `GET /friend/chat_session/list` | **核心！** 会话列表，AGENTS.md 声明在 friend.ts 里但没列函数 |
| B | `POST /friend/add_process` | 处理好友申请（同意/拒绝） |
| C | `POST /friend/add` | 发送好友申请 |
| D | `GET /friend/pending_event` | 待处理好友申请列表 |
| E | `POST /friend/remove` | 删除好友 |
| F | `GET /friend/search` | 搜索用户 |
| G | `POST /friend/chat_session/create` | 创建群聊 |
| H | `GET /friend/chat_session/member` | 获取会话成员 |
| I | `GET /user/multi_info` | 批量获取用户信息 |
| J | `POST /register` | 注册 |
| K | `GET /login/mail_verify_code` | 获取邮箱验证码 |
| L | `POST /user/avatar/set` | 设置头像（multipart） |
| M | `POST /user/nickname/set` | 改昵称 |
| N | `POST /user/description/set` | 改签名 |
| O | `GET /file/get_single` | 下载单个文件 |
| P | `GET /file/get_multi` | 批量下载文件 |

---

## 7. 修正后的契约表（建议替换 AGENTS.md §6.5）

```typescript
// ========== user.ts ==========
// 白名单接口（Authorization 头可选）
export function nicknameLogin(p: { nickname: string; password: string })
  : Promise<{ success?: boolean; errmsg?: string; token?: string }>
export function emailLogin(p: { email: string; verifyCode: string })
  : Promise<{ success?: boolean; errmsg?: string; token?: string }>
export function register(p: { nickname: string; password: string; email: string; verifyCode: string })
  : Promise<{ success?: boolean; errmsg?: string }>
export function getEmailVerifyCode(email: string)
  : Promise<{ success?: boolean; errmsg?: string }>

// 需鉴权接口
export function getProfile()
  : Promise<{ success?: boolean; errmsg?: string; userInfo: UserInfo }>
export function getMultiUserInfo(userIds: string[])
  : Promise<{ success?: boolean; errmsg?: string; usersInfo: Record<string, UserInfo> }>

// ========== message.ts ==========
export function getHistory(p: {
  chatSessionId: string;
  startTime: number;   // Unix 秒，内部 /60 转分钟拼 URL
  overTime: number;    // Unix 秒，内部 /60 转分钟拼 URL
}): Promise<{ success?: boolean; errmsg?: string; msgList: MessageInfo[] }>

export function getRecent(p: {
  chatSessionId: string;
  msgCount: number;    // ✅ 必填
  curTime?: number;    // Unix 秒，可选
}): Promise<{ success?: boolean; errmsg?: string; msgList: MessageInfo[] }>

export function searchMessage(p: {
  chatSessionId: string;
  searchKey: string;
}): Promise<{ success?: boolean; errmsg?: string; msgList: MessageInfo[] }>

export function publishMessage(body: PublishReq)
  : Promise<{ success?: boolean; errmsg?: string }>  // ✅ 不是 void

// ========== friend.ts ==========
export function getFriendList()
  : Promise<{ success?: boolean; errmsg?: string; friendList: UserInfo[] }>
export function getPendingFriendEvents()
  : Promise<{ success?: boolean; errmsg?: string; event: FriendEvent[] }>
export function getChatSessionList()
  : Promise<{ success?: boolean; errmsg?: string; chatSessionInfoList: ChatSessionInfo[] }>
export function addFriend(respondentId: string)
  : Promise<{ success?: boolean; errmsg?: string; notifyEventId?: string }>
export function processFriendApply(p: { notifyEventId: string; agree: boolean; applyUserId: string })
  : Promise<{ success?: boolean; errmsg?: string; newSessionId?: string }>

// ========== file.ts ==========
export function getSingleFile(fileId: string)
  : Promise<{ success?: boolean; errmsg?: string; fileData?: FileDownloadData }>
export function getMultiFile(fileIds: string[])
  : Promise<{ success?: boolean; errmsg?: string; fileData: Record<string, FileDownloadData> }>
```

---

## 8. AGENTS.md §5 / §6 需要同步修正的条目

| 位置 | 原文 | 修正为 |
|---|---|---|
| §5 第1条 | 字段 `messageId`、`chatSessionId`、`prevMessage` | ✅ 正确，proto3 JSON 就是 lowerCamelCase |
| §5 第2条 | `int64` 一律是字符串，展示：`dayjs.unix(Number(createTime))` | ⚠️ 正确。但**注意** HTTP 请求参数里的时间是 int64 分钟，不是 string |
| §5 第5条 | 时间格式化只允许 dayjs，禁止手写 new Date | ✅ 正确 |
| §6.3 | `mapError(401) → token 失效 + logout` | ✅ 正确，白名单接口以外 401 确实是 token 失效 |
| §6.4 | `POST /user/...`、`POST /file/...`、`GET /message/history\|recent\|search`、`POST /transmite/publish`、`/friend/...` | 🟡 路径基本正确，但具体函数签名见上方修正 |
| §6.6 消息发送流程 | 第3步「调 `publishMessage`」 | ✅ 路径正确 `/transmite/publish`，但注意请求体结构是 `{ chatSessionId, message: MessageContent }`，不是简单的 content |
| §6.6 第6条 | `publishMessage` 无返回值 | 🔴 错！真实返回 `{ success, errmsg }` |
| §6.7 | `FRIEND_APPLY_RESULT` 和 `CHAT_SESSION_CREATED` 推送 | ⚠️ 待验证，gateway.h 中未找到 |
| §10 第1条 | 历史接口时间参数是 Unix 分钟，闭区间 | ✅ 正确，见 gateway.h L518-528 |
| §10 第2条 | `/message/search` 无排序 | ✅ 正确 |
| §10 第3条 | 好友/群聊推送只带 id，收到后刷新列表 | ✅ 正确 |
