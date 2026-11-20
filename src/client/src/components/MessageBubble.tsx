import { useState } from "react";
import { AlertCircle, Image as ImageIcon } from "lucide-react";
import { useAuthStore } from "@/stores/auth";
import { useUserStore } from "@/stores/user";
import { Avatar } from "@/components/Avatar";
import { cn } from "@/components/ui/cn";
import { useAvatarUrl } from "@/hooks/useAvatarUrl";
import { imageMimeType, isImageFileName } from "@/lib/file";
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
  const fileMessage = msg.message.fileMessage;
  const isImage =
    fileMessage !== undefined && isImageFileName(fileMessage.fileName);
  const imageContent = fileMessage?.fileContent ?? "";
  // 历史消息只有 fileId，走预签名 URL；刚发送 / 推送的消息带 base64 内容，直接展示
  const imageUrl = useAvatarUrl(
    isImage && !imageContent ? fileMessage?.fileId : undefined,
  );
  const imageSrc = imageContent
    ? `data:${imageMimeType(fileMessage?.fileName)};base64,${imageContent}`
    : (imageUrl ?? "");
  const [imageBroken, setImageBroken] = useState(false);

  const content =
    fileMessage !== undefined
      ? `[文件] ${fileMessage.fileName ?? ""}`.trim()
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
            "rounded-base",
            isImage
              ? "overflow-hidden"
              : "whitespace-pre-wrap break-words px-3 py-2.5 text-base",
            isSelf ? "bg-bubble-self" : "bg-bubble-other shadow-sm",
          )}
          style={{ color: "var(--bubble-text)" }}
        >
          {isImage ? (
            imageSrc && !imageBroken ? (
              <img
                src={imageSrc}
                alt={fileMessage?.fileName ?? "图片"}
                className="block max-h-60 max-w-60 object-cover"
                loading="lazy"
                onError={() => setImageBroken(true)}
              />
            ) : (
              <div className="flex h-36 w-48 items-center justify-center text-tertiary">
                <ImageIcon size={28} strokeWidth={1.5} />
              </div>
            )
          ) : (
            content
          )}
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
