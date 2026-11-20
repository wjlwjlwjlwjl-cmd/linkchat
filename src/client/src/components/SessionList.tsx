import { Fragment, useMemo } from "react";
import dayjs from "dayjs";
import { hasUnread, useSessionStore } from "@/stores/session";
import { useUiStore } from "@/stores/ui";
import { Avatar } from "@/components/Avatar";
import { FriendSearch } from "@/components/FriendSearch";
import { cn } from "@/components/ui/cn";
import { isImageFileName } from "@/lib/file";
import type { ChatSessionInfo } from "@/types";

function formatTime(ts: string): string {
  return dayjs.unix(Number(ts)).format("HH:mm");
}

// 会话预览文案：文本取 content，图片固定 [图片]，其他文件显示文件名
function previewOf(session: ChatSessionInfo): string {
  const msg = session.prevMessage;
  if (!msg) return "";
  const file = msg.message.fileMessage;
  if (file) {
    return isImageFileName(file.fileName)
      ? "[图片]"
      : `[文件] ${file.fileName ?? ""}`.trim();
  }
  return msg.message.stringMessage?.content ?? "";
}

// 会话列表项（style.md §2.2）：高 64px，选中不加左侧绿条，只用底色
function SessionItem({
  session,
  pinned,
}: {
  session: ChatSessionInfo;
  pinned: boolean;
}) {
  const activeId = useSessionStore((s) => s.activeSessionId);
  const setActive = useSessionStore((s) => s.setActive);
  const cursor = useSessionStore((s) => s.readCursors[session.chatSessionId]);
  const active = activeId === session.chatSessionId;
  const unread = !active && hasUnread(session, cursor);
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
        active ? "bg-list-active" : cn(pinned && "bg-panel", "hv-list"),
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
        <div className="flex items-center gap-2">
          <p className="min-w-0 flex-1 truncate text-sm text-secondary">
            {previewOf(session)}
          </p>
          {/* 未读红点（style.md §2.2）：靠右下，8px，圆形 */}
          {unread && (
            <span
              aria-label="有新消息"
              className="h-2 w-2 shrink-0 rounded-full bg-danger"
            />
          )}
        </div>
      </div>
    </li>
  );
}

// 会话列表（style.md §2.2）：背景 --bg-list，顶部搜索框
export function SessionList() {
  const sessions = useSessionStore((s) => s.sessions);
  const pinnedIds = useSessionStore((s) => s.pinnedIds);
  const wsError = useUiStore((s) => s.wsError);

  // 置顶会话排在前面，其余保持后端顺序
  const { ordered, lastPinned } = useMemo(() => {
    const pinned = new Set(pinnedIds);
    const list = [...sessions].sort((a, b) => {
      const pa = pinned.has(a.chatSessionId) ? 0 : 1;
      const pb = pinned.has(b.chatSessionId) ? 0 : 1;
      return pa - pb;
    });
    let last = -1;
    list.forEach((s, i) => {
      if (pinned.has(s.chatSessionId)) last = i;
    });
    return { ordered: list, lastPinned: last };
  }, [sessions, pinnedIds]);

  return (
    <div className="flex h-full flex-col">
      {wsError && (
        <div
          role="alert"
          className="shrink-0 border-b border-danger-weak bg-danger-weak px-3 py-2 text-xs text-danger"
        >
          网络连接异常，请检查网络，正在重连…
        </div>
      )}
      <FriendSearch />
      <ul
        role="listbox"
        aria-label="会话列表"
        className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto"
      >
        {ordered.map((s, i) => (
          <Fragment key={s.chatSessionId}>
            <SessionItem
              session={s}
              pinned={pinnedIds.includes(s.chatSessionId)}
            />
            {i === lastPinned && (
              <li aria-hidden="true" className="border-b border-line" />
            )}
          </Fragment>
        ))}
      </ul>
    </div>
  );
}
