import dayjs from "dayjs";
import { Search } from "lucide-react";
import { useSessionStore } from "@/stores/session";
import { Avatar } from "@/components/Avatar";
import { cn } from "@/components/ui/cn";
import type { ChatSessionInfo } from "@/types";

function formatTime(ts: string): string {
  return dayjs.unix(Number(ts)).format("HH:mm");
}

// 会话预览文案：文本取 content，文件消息显示文件名
function previewOf(session: ChatSessionInfo): string {
  const msg = session.prevMessage;
  if (!msg) return "";
  if (msg.message.fileMessage) {
    return `[文件] ${msg.message.fileMessage.fileName ?? ""}`.trim();
  }
  return msg.message.stringMessage?.content ?? "";
}

// 会话列表项（style.md §2.2）：高 64px，选中不加左侧绿条，只用底色
function SessionItem({ session }: { session: ChatSessionInfo }) {
  const activeId = useSessionStore((s) => s.activeSessionId);
  const setActive = useSessionStore((s) => s.setActive);
  const active = activeId === session.chatSessionId;
  const name = session.chatSessionName ?? "未命名会话";
  const time = session.prevMessage
    ? formatTime(session.prevMessage.createTime)
    : "";

  return (
    <li
      role="option"
      aria-selected={active}
      tabIndex={0}
      onClick={() => setActive(session.chatSessionId)}
      className={cn(
        "flex h-16 cursor-pointer items-center px-3",
        active ? "bg-list-active" : "hv-list",
      )}
    >
      <div className="mr-3">
        <Avatar fileId={session.avatar} name={name} />
      </div>
      <div className="min-w-0 flex-1">
        <div className="flex items-center justify-between">
          <span className="truncate text-base text-primary">{name}</span>
          <span className="ml-2 shrink-0 text-xs text-tertiary tabular-nums">
            {time}
          </span>
        </div>
        <p className="truncate text-sm text-secondary">{previewOf(session)}</p>
      </div>
    </li>
  );
}

// 会话列表（style.md §2.2）：背景 --bg-list，顶部搜索框
export function SessionList() {
  const sessions = useSessionStore((s) => s.sessions);

  return (
    <div className="flex h-full flex-col">
      <div className="shrink-0 px-3 py-2">
        <div
          className="flex h-8 items-center rounded-base px-2"
          style={{ background: "var(--search-bg)" }}
        >
          <Search size={16} strokeWidth={1.5} className="text-secondary" />
          <input
            type="text"
            placeholder="搜索"
            className="ml-2 w-full bg-transparent text-sm text-primary placeholder:text-tertiary focus:outline-none"
          />
        </div>
      </div>
      <ul
        role="listbox"
        aria-label="会话列表"
        className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto"
      >
        {sessions.map((s) => (
          <SessionItem key={s.chatSessionId} session={s} />
        ))}
      </ul>
    </div>
  );
}
