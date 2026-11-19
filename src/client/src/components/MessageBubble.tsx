import { AlertCircle } from "lucide-react";
import { useAuthStore } from "@/stores/auth";
import { useUserStore } from "@/stores/user";
import { Avatar } from "@/components/Avatar";
import { cn } from "@/components/ui/cn";
import type { LocalMessage } from "@/types";

interface MessageBubbleProps {
  msg: LocalMessage;
  isGroup: boolean;
  compact?: boolean; // 与上一条同一人且间隔 ≤5 分钟时，间距缩小
  onRetry?: (msg: LocalMessage) => void;
}

// 消息气泡（style.md §2.4）：自己右绿 / 对方左白，4px 圆角，max-width 60%
export function MessageBubble({
  msg,
  isGroup,
  compact,
  onRetry,
}: MessageBubbleProps) {
  const myId = useAuthStore((s) => s.profile?.userId);
  const isSelf = myId !== undefined && msg.sender.userId === myId;

  // 消息 sender 只有 userId，昵称 / 头像从 user store 补全
  const otherUser = useUserStore((s) => s.users[msg.sender.userId]);
  const myName = useAuthStore((s) => s.profile?.nickname);
  const myAvatar = useAuthStore((s) => s.profile?.avatar);

  const nickname = isSelf
    ? (myName ?? "我")
    : (otherUser?.nickname ?? msg.sender.nickname ?? "未知用户");
  const avatarFileId = isSelf
    ? myAvatar
    : (otherUser?.avatar ?? msg.sender.avatar);

  // 实测：STRING 消息不返回 messageType，缺失时按文本处理
  const isFile = msg.message.fileMessage !== undefined;
  const content = isFile
    ? `[文件] ${msg.message.fileMessage?.fileName ?? ""}`.trim()
    : (msg.message.stringMessage?.content ?? "");

  return (
    <div
      className={cn(
        "flex w-full",
        isSelf ? "justify-end" : "justify-start",
        compact ? "mt-2" : "mt-4",
      )}
    >
      {!isSelf && (
        <div className="mr-3 mt-1">
          <Avatar fileId={avatarFileId} name={nickname} />
        </div>
      )}
      <div className="flex max-w-[60%] flex-col">
        {isGroup && !isSelf && (
          <span className="mb-1 text-xs text-secondary">{nickname}</span>
        )}
        <div
          className={cn(
            "break-words rounded-base px-3 py-2.5 text-base",
            isSelf ? "bg-bubble-self" : "bg-bubble-other shadow-sm",
          )}
          style={{ color: "var(--bubble-text)" }}
        >
          {content}
        </div>
      </div>
      {/* 发送状态（AGENTS.md §6.6）：sending 提示，failed 红叹号可点击重发 */}
      {isSelf && msg.status === "sending" && (
        <span className="mb-1 ml-2 self-end pb-1 text-xs text-tertiary">
          发送中
        </span>
      )}
      {isSelf && msg.status === "failed" && (
        <button
          type="button"
          aria-label="发送失败，点击重发"
          onClick={() => onRetry?.(msg)}
          className="mb-1 ml-2 self-end rounded-base p-0.5"
          style={{ color: "var(--color-danger)" }}
        >
          <AlertCircle size={16} strokeWidth={1.5} />
        </button>
      )}
      {isSelf && (
        <div className="ml-3 mt-1">
          <Avatar fileId={myAvatar} name={nickname} />
        </div>
      )}
    </div>
  );
}
