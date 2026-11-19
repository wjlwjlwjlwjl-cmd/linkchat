# AGENTS.md — LinkChat Web 前端

本文件约束所有在本目录（`src/client`）内产生的代码。**视觉规范以 [style.md](./style.md) 为准，工程规范以本文件为准；两者冲突时，视觉问题看 style.md，技术问题看本文件。**

**本文件同时是 AI 协作的操作手册。** §0 的执行协议凌驾于所有其他条款；违反 §0 的产出即使代码正确也不合规范。

---

## 0. AI 任务执行协议（最高优先级）

每次接活按此流程，不得跳步：

1. **复述 + 列文件**：先用一句话复述任务，然后列出将改动的文件（新增 / 修改分开列），**等我确认后再动手**。确认前不得写任何代码。
2. **只改已确认的文件**：需要触碰以下任一内容时，停下来单独申请，不得顺手改：
   - 新增 / 删除 `src/` 下的平级目录；
   - 修改 `tokens.css` 中任何已有 token 的值；
   - 新增依赖；
   - 改动 `tailwind.config.ts` 的 theme 桥接；
   - 任何涉及 `ws/` 重连、心跳、去重逻辑的改动。
3. **整文件交付**：每完成一个文件，给出**完整文件内容**（不要 diff 片段、不要"其余不变"），便于整文件覆盖。
4. **不确定就问**：后端字段、接口行为、交互细节不确定时先问，不要猜。若因阻塞必须猜，代码里标 `// TODO(确认): ...`，并在交付说明里列出。
5. **交付前自检**：逐条对照 §9 DoD 输出自检表（哪条满足、哪条没做及原因）。自检表是交付的一部分，不得省略。

---

## 1. 项目定位

- LinkChat 是学生个人练手项目，非商业、非公网部署。
- 对外只称 `LinkChat`，不得出现「微信 / WeChat」字样，不得使用其 Logo、截图、图标切图或官方字体（详见 style.md §0）。
- 品牌色使用 **`#22C55E`**（style.md 推荐的 B 方案），所有 UI 仅作布局演示。

---

## 2. 技术栈（已锁定，不得替换）

| 层 | 选型 | 版本约束 |
| --- | --- | --- |
| 包管理 | pnpm | Node 20 LTS |
| 构建 | Vite | v6 |
| 框架 | React + TypeScript | React 18，`strict: true` |
| 样式 | Tailwind CSS | **3.4.x**（禁止 v4） |
| 组件基元 | shadcn/ui | 源码落在 `src/components/ui`，可直接改 |
| 客户端状态 | Zustand | v5 |
| 服务端数据 | TanStack Query | v5 |
| 路由 | react-router-dom | v6 |
| 长列表 | react-virtuoso | 消息列表、会话列表超过 100 条时使用 |
| 图标 | lucide-react | 24px、1.5px 描边 |
| 时间 | dayjs | 仅用于展示格式化 |
| WebSocket | 原生 `WebSocket` | 自行封装，不引第三方 |
| 工具 | clsx + tailwind-merge | 统一用 `cn()` |

**禁止引入**：Next.js、Nuxt、Electron、Ant Design、MUI、Element Plus、Tailwind v4、axios（用原生 fetch 封装）、moment、lodash 全量包。

新增任何依赖前，先说明理由并控制在 1 个以内；能用现有栈解决就不要加包。

---

## 3. 目录结构（固定，不得新增平级目录）

```
src/client/
├── AGENTS.md
├── style.md
├── index.html
├── vite.config.ts
├── tailwind.config.ts
├── tsconfig.json
└── src/
    ├── main.tsx
    ├── App.tsx
    ├── styles/
    │   └── tokens.css          # style.md 中全部 CSS 变量，唯一色值来源
    ├── api/
    │   ├── client.ts           # 唯一 HTTP 出口（见 §6.2）
    │   ├── error.ts            # 错误码 → UI 映射（见 §6.3）
    │   ├── mock.ts             # 与真实 api 同签名的假数据（见 §3.1）
    │   ├── user.ts / file.ts / message.ts / friend.ts / transmite.ts
    ├── ws/
    │   ├── client.ts           # 连接 / 心跳 / 重连
    │   ├── dispatcher.ts       # type → handler 映射表
    │   └── handlers/           # 每个推送 type 一个文件
    ├── types/                  # 所有接口类型（禁止 any）
    ├── stores/                 # Zustand：session / message / friend / ui
    ├── features/               # 业务模块：auth、chat、contacts、settings
    ├── components/             # 通用组件 + ui/（shadcn 生成）
    └── hooks/                  # 可复用 hooks
```

职责边界：

- `api/` 只做「请求 + 返回类型」，不含状态，不操作 store。
- `ws/` 只负责连接生命周期和把推送交给 `stores/`，不写 UI 逻辑。
- `features/` 内可组合 `api`、`stores`、`components`，页面级逻辑放这里。
- `components/` 不得 import `features/`。
- 组件文件一文件一组件，文件名用 `PascalCase.tsx`；非组件文件用 `camelCase.ts`。

### 3.1 Mock 策略

- 后端未就绪时，`api/mock.ts` 必须导出与真实 `api/*.ts` **同函数名、同签名、同返回类型**的假实现。
- 切换靠 `VITE_USE_MOCK=true`，`api/*.ts` 内部统一 `if (USE_MOCK) return mockXxx(...)`，**组件层无感知**。
- 禁止在组件或 store 里写死假数据；禁止 mock 函数返回 `any`。

---

## 4. 视觉硬约束（摘要，细节见 style.md）

1. **只允许使用 token**：颜色、圆角、间距、阴影一律 `var(--token)` 或 Tailwind 中桥接的 token 类名，禁止出现新的 hex / rgba 字面量。
   **豁免三类**：`src/styles/tokens.css` 本身、`src/components/ui/**`（shadcn 生成的原始文件）、图片占位背景。
2. token 只定义在 `src/styles/tokens.css` 的 `:root`；新增样式先补 token，再写组件。
3. 关键尺寸：功能栏 60px、会话列表 260px（拖拽 220–360）、标题栏 60px、输入工具栏 40px、头像 40px。
4. 气泡：`4px` 小圆角、最大宽度 `60%`、自己 `--bubble-self` / 对方 `--bubble-other`；**不使用大圆角、不加重阴影**。
5. 列表项选中**不加左侧绿条**，只用底色；栏间分隔只用 1px `--line`，不用阴影。
6. `html, body { height: 100%; overflow: hidden; }`；滚动容器必须 `min-height: 0` + `overscroll-behavior: contain`。
7. hover 样式用 `@media (hover: hover)` 包裹；聚焦必须保留 `outline`，用 `--focus-ring`。
8. 输入框 `Enter` 发送、`Shift+Enter` 换行；**IME 组合期间（compositionstart→compositionend）Enter 不发送**。
9. 小于 `1024px` 只显示「请放大窗口」提示页，不做移动端适配。
10. 无障碍底线（示例见下）：

```tsx
// 会话列表
<ul role="listbox" aria-label="会话列表">
  <li role="option" aria-selected={activeId === s.id} tabIndex={0}>...</li>
</ul>

// 消息区
<div role="log" aria-live="polite">...</div>

// 未读角标
<span aria-label={`${count} 条未读`}>{count > 99 ? '99+' : count}</span>
```

### 4.1 Tailwind token 桥接示例

`tailwind.config.ts` 的 `theme.extend` 只读 CSS 变量，**不写死色值**：

```ts
theme: {
  extend: {
    colors: {
      brand: 'var(--color-brand)',
      app: 'var(--bg-app)',
      list: 'var(--bg-list)',
      panel: 'var(--bg-panel)',
      primary: 'var(--text-primary)',
      secondary: 'var(--text-secondary)',
      line: 'var(--line)',
    },
    borderRadius: {
      base: 'var(--radius-base)',
      lg: 'var(--radius-lg)',
      pill: 'var(--radius-pill)',
    },
  },
}
```

---

## 5. 类型与数据约定

1. **后端返回的是 proto3 JSON**：字段为 lowerCamelCase（`messageId`、`chatSessionId`、`prevMessage`）。
2. **`int64` 一律是字符串**：`createTime` 声明为 `string`。
   - 比较 / 排序前必须 `Number(createTime)`；
   - **禁止** `createTime > 0`、`Number(x) > Date.now()` 这类混用秒 / 毫秒的写法；
   - 展示唯一写法：`dayjs.unix(Number(createTime)).format('HH:mm')`。
3. 所有接口响应类型集中在 `src/types/`，函数签名必须显式标注，禁止 `any` 和 `as unknown as`。
4. 消息去重以 `messageId` 为唯一键；排序以 `Number(createTime)`，相同时间再按 `messageId` 稳定排序。
5. 时间格式化只允许 `dayjs`；禁止手写 `new Date` 拼接。

---

## 6. API 与 WebSocket 约定

### 6.1 基地址与鉴权

- `VITE_API_BASE` 默认 `http://127.0.0.1:9000`；`VITE_WS_BASE` 默认 `ws://127.0.0.1:9001`。
- **URL 拼接规则**：`${VITE_API_BASE}${path}`，`path` 以 `/` 开头且**不含 `/api` 前缀**。
- HTTP 头 `Authorization: <token>`（**不加 `Bearer ` 前缀**）；WS 用 `ws://host:9001/?token=<token>`。

### 6.2 HTTP 出口（唯一，人定死，AI 不得改签名）

```ts
// api/client.ts
export function request<T>(path: string, init?: RequestInit): Promise<T>
// 内部负责：拼 base、加 Authorization、401 清 token 跳登录、非 2xx 抛 ApiError
```

### 6.3 错误码映射（`api/error.ts`）

```ts
export function mapError(code: number): { message: string; action?: 'logout' | 'retry' }
// 400 → 参数错误 / 401 → token 失效 + logout / 502 → 后端不可用 + retry
```

所有 UI 错误提示必须经 `mapError`，禁止各组件自造文案。

### 6.4 已存在的 HTTP 端点（路径无 `/api` 前缀）

- `POST /user/...`、`POST /file/...`、`GET /message/history|recent|search`、`POST /transmite/publish`、`/friend/...`
- `/message/recent` 的 `cur_time` 可省略，服务端默认取当前时间。

### 6.5 接口契约表（函数名与签名是契约，实现可后补）

```ts
// api/message.ts
export function getHistory(p: { sessionId: string; startTime: string; endTime: string }): Promise<MessageInfo[]>
export function getRecent(p: { sessionId: string; curTime?: string }): Promise<MessageInfo[]>
export function searchMessage(p: { sessionId: string; keyword: string }): Promise<MessageInfo[]>
export function publishMessage(body: PublishReq): Promise<void>   // 注意：无返回值，拿不到服务端 id

// api/user.ts
export function login(p: { username: string; password: string }): Promise<{ token: string }>
export function getProfile(): Promise<UserInfo>

// api/friend.ts
export function getFriendList(): Promise<FriendInfo[]>
export function getApplyList(): Promise<ApplyInfo[]>
```

**store action 契约**（`stores/message.ts`）：

```ts
appendOptimistic(sessionId: string, msg: LocalMessage): void
markFailed(sessionId: string, tempId: string): void
dedupeById(sessionId: string, incoming: MessageInfo[]): void
```

组件、store、api 三层必须按此对号，不得自创函数名。

### 6.6 消息发送流程（写死，不得自创）

1. 生成临时 id：`temp-${crypto.randomUUID()}`。
2. 乐观插入 store，`status: 'sending'`，`createTime` 用 `dayjs().unix().toString()`。
3. 调 `publishMessage`。
4. **失败**：`markFailed(sessionId, tempId)`，UI 显示红色感叹号 + 可点击重发。
5. **成功**：`status: 'sent'`。**不替换 id**——自己发送的消息收不到 `NEW_MESSAGE` 推送，拿不到服务端 `messageId`，因此临时 id 保留，去重逻辑**只对服务端推送的 `messageId` 生效**，不与临时 id 混用。

### 6.7 WS 推送信封

`{"type": "...", "data": {...}}`，text 帧，`type` 取值：

- `NEW_MESSAGE`：`data` 为完整 MessageInfo（含 `createTime`）
- `FRIEND_REQUEST`：`{notifyEventId, applyUserId}`
- `FRIEND_APPLY_RESULT`：`{agree, newSessionId?}`
- `CHAT_SESSION_CREATED`：`{chatSessionId, chatSessionName, creatorId}`

### 6.8 WS 模块结构（固定）

```
ws/client.ts       # 连接、30s 心跳、指数退避重连（1s→2s→4s…上限 30s）
ws/dispatcher.ts   # export const handlers: Record<MsgType, Handler>
ws/handlers/newMessage.ts / friendRequest.ts / friendApplyResult.ts / chatSessionCreated.ts
```

- 重连成功后对当前会话拉一次 `/message/recent` 补齐并用 `messageId` 去重。
- 所有推送处理集中在 `dispatcher.ts`，组件内**不得** `addEventListener('message')`。

---

## 7. 状态管理规则

1. 组件不直接调用 `fetch` 或 `WebSocket`；只能调用 `api/` 函数或 `stores/` 的 action。
2. 消息按会话存 `Map<sessionId, Message[]>`；切换会话不得清空已有数据。**MVP 不设缓存上限**（如后续要加上限，先申请）。
3. 未读数、最后一条消息、在线状态统一放 store，禁止组件间通过 props 层层传递。
4. TanStack Query 只负责「拉取类」接口；WS 推送结果直接写 Zustand，不要反向塞进 Query 缓存。

---

## 8. 代码质量门禁

提交前必须全部通过：

```bash
pnpm tsc --noEmit
pnpm eslint . --max-warnings 0
pnpm prettier --check .
pnpm build
```

约定：

- 组件使用具名导出；`App`、页面入口除外。
- import 使用 `@/` 别名指向 `src/`，禁止 `../../../`。
- 不允许留 `console.log`（调试用 `console.debug` 也需删除）。
- 每个新组件 / 函数写**一行** `//` 用途注释即可，禁止 JSDoc 大段。
- UI 改动必须自检 1024×640 与 1280×800 两个尺寸不溢出（截图或人工确认）。

---

## 9. 完成定义（DoD）

一个改动算完成，需同时满足：

1. `pnpm build` 成功，`tsc` / `eslint` / `prettier` 通过；
2. 视觉上只使用 token，未引入新色值（§4 豁免项除外）；
3. 交互覆盖以下可观察行为：
   - [ ] 正常态
   - [ ] hover **仅在支持 hover 的设备生效**（`@media (hover: hover)`）
   - [ ] focus 有**可见** outline（`--focus-ring`）
   - [ ] 空态
   - [ ] 错误态（经 `mapError` 的统一文案）
4. 涉及接口时：类型完整、`createTime` 按字符串处理、走 `api/client.ts`；
5. 涉及推送时：`ws/dispatcher.ts` 有对应分支、handler 文件存在、store 行为明确；
6. 页面在 1024×640 与 1280×800 下不出现布局溢出。

---

## 10. 已知后端注意点（避免重复踩坑）

- 历史接口时间参数是 **Unix 分钟**，且为闭区间（注意与 `createTime` 的 Unix 秒区分）；
- `GET /message/search` 结果无显式排序，前端需自行按 `Number(createTime)` 排序；
- 好友 / 群聊推送只带 id 与事件字段，收到后需要时再调对应列表接口刷新。

---

## 11. 里程碑（M0 先行）

**M0 骨架**（任何业务逻辑之前必须先完成）：

- 三栏布局静态可跑，token 全部落地；
- 假数据渲染会话列表 + 消息列表；
- 1024×640 / 1280×800 不溢出；
- **不含任何真实接口与 WebSocket**。

M0 验收通过后才进入业务模块（auth → chat → contacts → settings）。任何跳过 M0 直接写业务的产出视为不合规。