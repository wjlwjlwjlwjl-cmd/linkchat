# LinkChat Web 前端视觉与布局规范

---

## 0. 前置约束（合规与命名）

**本项目为学生个人练手，非商业、非公网部署、不收集真实用户数据。以下为硬约束，违反则不得对外展示。**

- **项目名不得含"微信 / WeChat / 微信桌面端"**。对外统一称 `LinkChat`，描述写"参考某 IM 桌面端的三栏布局"。
- **禁止使用**：微信 Logo、界面截图、官方图标字体、`WeChat` 字样、启动图/登录页插画等品牌识别元素。
- **图标全部来自开源图标库**（推荐 lucide，24px 线性、1.5px 描边），不使用微信官方 SVG/PNG 切图。
- **品牌色二选一**：
  - A. 保留 `#07C160`（仅作布局演示色），README 必须注明"颜色仅用于布局演示，与任何商业产品无关"；
  - B. 换成近似但不同的绿，如 `#22C55E`，降低识别度。**推荐 B。**
- 头像占位、群头像拼图由前端生成，不引用外部图片；表情先用 Unicode Emoji。

---

## 1. 设计令牌（Design Tokens）

实现时统一落到 CSS 变量（`:root`），组件内禁止写死颜色。

### 1.1 品牌色

| Token | 值 | 用途 |
| --- | --- | --- |
| `--color-brand` | `#22C55E`（或 `#07C160`，见 0 节） | 主按钮、选中态、发送按钮、未读点 |
| `--color-brand-hover` | `#1EAE53` | 主按钮 hover |
| `--color-brand-active` | `#199A49` | 主按钮 pressed |
| `--color-brand-soft` | `rgba(34, 197, 94, 0.12)` | 浅绿标签底 |
| `--color-link` | `#576B95` | 文本中的链接、可点击的次要操作 |
| `--focus-ring` | `2px solid var(--color-brand)` | 统一聚焦环，`outline-offset: 2px` |

### 1.2 中性色（浅色主题）

| Token | 值 | 用途 |
| --- | --- | --- |
| `--bg-app` | `#F5F5F5` | 聊天区背景 |
| `--bg-rail` | `#2E2E2E` | 最左侧功能栏 |
| `--bg-rail-hover` | `#3A3A3A` | 功能栏 hover |
| `--bg-rail-active` | `#454545` | 功能栏选中 |
| `--bg-list` | `#F7F7F7` | 会话/联系人列表 |
| `--bg-list-hover` | `#EDEDED` | 列表项 hover |
| `--bg-list-active` | `#E3E3E3` | 列表项选中 |
| `--bg-panel` | `#FFFFFF` | 卡片、气泡、弹层、输入框 |
| `--bg-header` | `#F5F5F5` | 会话顶部栏 |
| `--bg-input` | `#FFFFFF` | 输入区 |
| `--bg-tooltip` | `#4C4C4C` | Tooltip / Toast 深色底 |
| `--line` | `#E7E7E7` | 主要分割线 |
| `--line-light` | `#EDEDED` | 次级分割线 |

### 1.3 文本色

| Token | 值 | 用途 |
| --- | --- | --- |
| `--text-primary` | `#191919` | 正文、标题 |
| `--text-secondary` | `#767676` | 时间、摘要、辅助信息（对比度 ≥ 4.5:1） |
| `--text-tertiary` | `#B2B2B2` | 占位符、禁用文本（仅非关键信息） |
| `--text-inverse` | `#FFFFFF` | 深色底上的文字 |
| `--text-on-brand` | `#FFFFFF` | 主色按钮文字 |

### 1.4 消息气泡

| Token | 值 | 说明 |
| --- | --- | --- |
| `--bubble-self` | `#95EC69` | 自己发送 |
| `--bubble-other` | `#FFFFFF` | 对方发送 |
| `--bubble-text` | `#191919` | 气泡内文字 |
| `--bubble-radius` | `4px` | 微信风格小圆角，不做大圆角 |
| `--bubble-max-width-ratio` | `60%` | 相对**消息区行容器**的最大宽度（CSS 中用于气泡 `max-width: 60%`） |
| `--bubble-padding` | `10px 12px` | 上下 / 左右 |

### 1.5 功能色

| Token | 值 | 用途 |
| --- | --- | --- |
| `--color-danger` | `#FA5151` | 错误、未读红点、删除 |
| `--color-warning` | `#FA9D3B` | 警告 |
| `--color-info` | `#10AEFF` | 提示、链接型提示 |
| `--color-success` | `var(--color-brand)` | 成功 |

### 1.6 字体

```css
--font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "PingFang SC",
               "Microsoft YaHei", "Helvetica Neue", Arial, sans-serif;
```

| Token | 值 | 用途 |
| --- | --- | --- |
| `--font-size-xs` | `12px` | 时间戳、角标 |
| `--font-size-sm` | `13px` | 列表摘要、次要信息 |
| `--font-size-base` | `14px` | 正文、消息、输入框 |
| `--font-size-lg` | `16px` | 会话标题 |
| `--font-size-xl` | `20px` | 空状态标题（仅空状态/引导页使用） |
| `--line-height` | `1.5` | 全局正文行高 |

- 时间、未读数使用 `font-variant-numeric: tabular-nums;` 防止抖动。
- 中文不要加 `letter-spacing`；英文标题可加 `0.2px`。
- 字体渲染：`body { -webkit-font-smoothing: antialiased; }`，并在 README 注明"macOS 开启、Windows 无影响"。

### 1.7 间距、圆角、阴影

| Token | 值 |
| --- | --- |
| `--space-0` ~ `--space-8` | `0 / 4 / 8 / 12 / 16 / 20 / 24 / 32px` |
| `--radius-sm` | `2px`（小标签） |
| `--radius-base` | `4px`（头像、气泡、输入框） |
| `--radius-lg` | `6px`（弹层、菜单） |
| `--radius-pill` | `999px`（未读角标、胶囊按钮） |
| `--shadow-sm` | `0 1px 2px rgba(0, 0, 0, 0.06)` |
| `--shadow-popup` | `0 4px 16px rgba(0, 0, 0, 0.12)` |
| `--shadow-toast` | `0 2px 8px rgba(0, 0, 0, 0.16)` |

### 1.8 尺寸

| Token | 值 | 说明 |
| --- | --- | --- |
| `--rail-width` | `60px` | 左侧功能栏 |
| `--list-width` | `260px` | 会话列表，可拖拽 220–360 |
| `--titlebar-height` | `40px` | **预留，Web 端不实现** |
| `--header-height` | `60px` | 会话标题栏 |
| `--toolbar-height` | `40px` | 输入区工具栏 |
| `--input-min-height` | `120px` | 输入区总高 |
| `--avatar-size` | `40px` | 列表与消息头像 |
| `--avatar-radius` | `4px` | 方头像小圆角 |
| `--badge-height` | `18px` | 未读角标 |

### 1.9 动效

| Token | 值 |
| --- | --- |
| `--ease-standard` | `cubic-bezier(0.2, 0, 0, 1)` |
| `--duration-fast` | `120ms` |
| `--duration-base` | `200ms` |

- hover / 选中态统一 `120ms`；弹层出现 `200ms`，只做透明度 + 2px 位移，不做弹跳。
- 无障碍：`@media (prefers-reduced-motion: reduce) { --duration-fast: 0ms; --duration-base: 0ms; }`

---

## 2. 整体布局

```
┌────┬──────────────┬───────────────────────────────────────┬ ─ ─ ─ ─ ─ ┐
│    │              │  会话标题（60px）                      │           │
│ 功 │  搜索框      ├───────────────────────────────────────┤  可选     │
│ 能 │              │                                       │  详情     │
│ 栏 │  会话列表    │  消息区（可滚动）                      │  侧栏     │
│60px│  260px       │                                       │  300px    │
│    │              ├───────────────────────────────────────┤           │
│    │              │  输入工具栏（40px）                    │           │
│    │              │  输入框（自适应，最小 120px）          │           │
└────┴──────────────┴───────────────────────────────────────┴ ─ ─ ─ ─ ─ ┘
```

- 应用最小尺寸 `1024 × 640`，默认 `1280 × 800`。**低于 1024 宽显示"请放大窗口"提示页，不做移动端降级。**
- 三栏高度全部 `100%`，只有「消息区」和「会话列表」各自独立滚动，页面本身不滚动。
- **落地要求**：`html, body { height: 100%; overflow: hidden; }`；三栏 flex 容器中，滚动子项必须 `min-height: 0`，否则 `overflow` 失效。
- 滚动容器加 `overscroll-behavior: contain`，防止触控板滚动穿透。
- 栏与栏之间用 `1px solid var(--line)` 分隔，不用阴影。
- 图片全局 `img { user-select: none; -webkit-user-drag: none; }`，防止拖拽破坏布局。

### 2.1 左侧功能栏

- 深色底 `--bg-rail`，图标 24×24，水平居中，垂直间距 24px。
- 顶部：本人头像（32px，圆角 4px），底部：设置。
- 选中项：图标 `--text-inverse`，背景 `--bg-rail-active`；未选中：`rgba(255,255,255,0.75)`。
- 未读提示：右上角红点，直径 8px，用 `box-shadow: 0 0 0 1.5px var(--bg-rail)` 做描边（避免 1px 屏糊）。

### 2.2 会话列表

- 背景 `--bg-list`；顶部搜索框高 32px、圆角 4px、底色 `#E9E9E9`，聚焦时白底 + `--color-brand` 1px 边框 + `--focus-ring`。
- 列表项高 64px，内边距 `12px 12px`，结构：头像 40px + 右侧两行（标题 14px / 摘要 13px 灰）。
- 右上角时间 11px `--text-tertiary`；未读角标靠右下。
- hover `--bg-list-hover`，选中 `--bg-list-active`；选中项**不加**左侧绿条，只用底色区分。
- 列表项之间无分割线，靠间距区分。

### 2.3 会话标题栏

- 高 60px，底色 `--bg-header`，底部 1px `--line-light`。
- 左侧：会话名（16px，medium），群聊可带「(成员数)」后缀（13px 灰）。
- 右侧：搜索、更多（24px 图标，hover 显示 4px 圆角浅灰底）。
- 点击标题可展开会话详情侧栏（宽 300px，从右侧滑入；≥1280px 常驻，否则浮层）。

### 2.4 消息区

- 背景 `--bg-app`，左右内边距 20px，上下 16px。
- 消息之间垂直间距 16px；**连续同一人**且距**上一条消息**≤5 分钟可缩到 8px。
- 时间分割线：居中，12px `--text-tertiary`，格式 `HH:mm` 或 `昨天 HH:mm`；以**该分组最后一条消息**为基准，间隔 >5 分钟插入新分割线。
- 对方消息：头像在左，气泡白色 + `--shadow-sm`；自己消息：头像在右，气泡绿色。
- 气泡内文字 14px / 1.5，超长单词 `word-break: break-word`。
- 图片消息：最大 240×240，圆角 4px，点击查看大图。
- 文件消息：白卡片，图标 + 文件名 + 大小，hover 整卡变 `#FAFAFA`。
- 系统提示（入群、撤回）：居中，12px，`--text-tertiary`，底色 `rgba(0,0,0,0.04)`，圆角 2px，内边距 `2px 8px`。

### 2.5 输入区

- 顶部 1px `--line-light`，背景 `--bg-input`。
- 工具栏：表情、图片、文件、截图（图标 20px，间距 16px，hover 变 `--color-brand`）。
- 文本域：无边框，14px，占满剩余高度，`resize: none`；占位符 `--text-tertiary`。
- 右下角发送按钮：`88 × 28`，`--color-brand`，白字 13px，圆角 4px；`Enter` 发送、`Shift + Enter` 换行；**IME 组合期间（`compositionstart` 到 `compositionend`）`Enter` 不发送**；禁用时 `#E9E9E9` 底 + `--text-tertiary` 字。
- 粘贴：第一版不做图片/文件粘贴，粘贴纯文本即可；后续可扩展。

### 2.6 空状态

- 居中，图标 64px（`#D8D8D8`），标题 20px `--text-secondary`，说明 13px `--text-tertiary`。
- 文案示例：「选择一个会话开始聊天」。

---

## 3. 组件规范

### 3.1 头像

- 方形 40×40，圆角 4px；群聊九宫格：**2 人左右分屏、3–4 人 2×2、5–9 人 3×3**。
- 无头像时显示昵称首字，底色从固定色板中按 `user_id` 哈希取（禁用随机）。
- 头像右上角在线状态点：8px，`--color-brand`，2px 白描边（用 `box-shadow` 实现）。

### 3.2 按钮

| 类型 | 样式 |
| --- | --- |
| 主按钮 | `--color-brand` 底、白字、圆角 4px、高 28/32px |
| 次按钮 | 白底、1px `--line`、`--text-primary` |
| 文本按钮 | 无底、`--color-link` |
| 危险按钮 | `--color-danger` 底、白字 |

禁用：`opacity: 0.5`，`cursor: not-allowed`，不加额外描边。

### 3.3 弹层 / 菜单

- 白底、`--radius-lg`、`--shadow-popup`、内边距 4px。
- 菜单项高 32px、左右 12px、13px 字；hover `#F2F2F2`；危险项文字 `--color-danger`。
- 出现动画：`opacity 0→1` + `translateY(-2px→0)`，200ms。
- **焦点管理**：打开时焦点陷阱（focus trap），`Esc` 关闭，关闭后焦点回到触发元素。

### 3.4 未读角标

- 高 18px，最小宽 18px，胶囊形，`--color-danger` 底，白字 11px，`padding: 0 5px`。
- 超过 99 显示 `99+`。
- 可访问性：`aria-label="N 条未读"`。

### 3.5 滚动条

```css
.scroll-area { opacity: 0; transition: opacity var(--duration-fast); }
.scroll-area:hover { opacity: 1; }

::-webkit-scrollbar { width: 6px; height: 6px; }
::-webkit-scrollbar-thumb { background: rgba(0, 0, 0, 0.15); border-radius: 3px; }
::-webkit-scrollbar-thumb:hover { background: rgba(0, 0, 0, 0.25); }
::-webkit-scrollbar-track { background: transparent; }

/* Firefox 降级 */
@supports not selector(::-webkit-scrollbar) {
  .scroll-area { opacity: 1; scrollbar-width: thin; }
}
```

滚动条默认不可见，hover 容器时淡入（通过容器 `opacity` 实现，而非直接改 `::-webkit-scrollbar`）。

---

## 4. 交互与状态

| 场景 | 规范 |
| --- | --- |
| 列表项 hover | `--bg-list-hover`，120ms；**用 `@media (hover: hover)` 包裹** |
| 图标按钮 hover | 4px 圆角、`rgba(0,0,0,0.05)` 底；同样 `@media (hover: hover)` |
| 聚焦 | `outline: var(--focus-ring); outline-offset: 2px;` **禁止移除 outline** |
| 消息发送中 | 气泡右侧显示 14px 转圈，失败显示红色感叹号，可点击重发 |
| 新消息到达 | 若用户已在底部则自动滚动到底；否则显示「N 条新消息」浮层按钮 |
| 切换会话 | 消息区立即切换，列表选中态同步，滚动位置按会话记忆；**焦点落到输入框** |
| 拖拽分栏 | 列表右侧 4px 命中区，`cursor: col-resize`，宽度限制 220–360px |

键盘：

- `Enter` 发送（**IME 组合期间不发送**），`Shift + Enter` 换行；
- `Esc` 关闭弹层；
- `↑` 在空输入框时编辑最后一条自己发送的消息（与微信一致，需处理 IME）；
- `Ctrl/Cmd + F` 聚焦搜索。

ARIA：

- 会话列表：`role="listbox"`，列表项 `role="option"` + `aria-selected`；
- 消息区：`role="log"` + `aria-live="polite"`；
- 未读角标：`aria-label`。

---

## 5. 深色主题（可选，第二阶段）

通过 `[data-theme="dark"]` 覆盖同名变量，不改组件。**下表为完整覆盖列表，未列出的变量继承浅色。**

| Token | 深色值 |
| --- | --- |
| `--bg-app` | `#111111` |
| `--bg-rail` | `#1A1A1A` |
| `--bg-list` | `#191919` |
| `--bg-list-hover` | `#232323` |
| `--bg-list-active` | `#2B2B2B` |
| `--bg-panel` | `#1E1E1E` |
| `--bg-header` | `#191919` |
| `--bg-input` | `#1E1E1E` |
| `--bg-tooltip` | `#3A3A3A` |
| `--line` | `#2A2A2A` |
| `--line-light` | `#232323` |
| `--text-primary` | `#E5E5E5` |
| `--text-secondary` | `#9A9A9A` |
| `--text-tertiary` | `#6B6B6B` |
| `--bubble-self` | `#2C5D3F` |
| `--bubble-other` | `#2C2C2C` |
| `--bubble-text` | `#E5E5E5` |
| `--color-brand` | `#22C55E`（深色下提升亮度） |
| `--color-link` | `#7A9CC6` |

---

## 6. 响应式

| 宽度 | 行为 |
| --- | --- |
| ≥ 1280px | 三栏 + 详情侧栏常驻 |
| 1024–1279px | 三栏，详情侧栏改浮层（点击标题打开） |
| < 1024px | **显示"请放大窗口至 1024×640 以上"提示页**，不实现移动端降级 |

本次目标为桌面 Web，`<1024px` 只保证不崩，不做完整交互。

---

## 7. 资源

- 图标统一 lucide（24px，1.5px 描边）。
- 头像占位、群头像拼图由前端生成，不引用外部图片。
- 表情先用 Unicode Emoji。
- 所有图片资源走 `object-fit: cover`，加载中显示 `#EDEDED` 骨架。

---

## 8. 实现约定

- 颜色、圆角、间距、阴影只能引用变量，禁止在组件里写 `#hex`（图片背景色除外）。
- 尺寸使用 `px`，不做 `rem` 缩放；字体不随视口变化。
- 任何新增视觉样式先补 token，再写组件。
- 技术栈建议（参考，不强制）：Vite + React/Vue + CSS Modules，或 Tailwind 通过 `theme.extend` 桥接上述 CSS 变量；不用组件库默认皮肤，避免覆盖成本。