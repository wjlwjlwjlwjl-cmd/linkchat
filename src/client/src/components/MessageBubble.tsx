import { useState } from "react";
import { AlertCircle, FileText, Image as ImageIcon } from "lucide-react";
import { getSingleFile } from "@/api/file";
import { ImageViewerDialog } from "@/components/ImageViewerDialog";
import { useAuthStore } from "@/stores/auth";
import { useUserStore } from "@/stores/user";
import { Avatar } from "@/components/Avatar";
import { UserInfoPopup } from "@/components/UserInfoPopup";
import { cn } from "@/components/ui/cn";
import { useFileUrlStore } from "@/stores/fileUrl";
import { imageMimeType, isImageFileName, formatFileSize } from "@/lib/file";
import type { LocalMessage, UserInfo } from "@/types";

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
  const [infoAnchor, setInfoAnchor] = useState<{
    userId: string;
    rect: DOMRect;
    initial: UserInfo;
  } | null>(null);

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
  // 非图片文件：以文件卡片展示
  const isFile = fileMessage !== undefined && !isImage;
  const fileSizeText = formatFileSize(Number(fileMessage?.fileSize ?? 0));
  const imageContent = fileMessage?.fileContent ?? "";
  // 历史消息只有 fileId，地址由消息列表的批量预签名接口填充；刚发送 / 推送的消息带 base64 内容，直接展示
  const imageUrl = useFileUrlStore((s) =>
    isImage && !imageContent && fileMessage?.fileId
      ? (s.urls[fileMessage.fileId]?.url ?? "")
      : "",
  );
  const imageSrc = imageContent
    ? `data:${imageMimeType(fileMessage?.fileName)};base64,${imageContent}`
    : (imageUrl ?? "");
  const [imageBroken, setImageBroken] = useState(false);
  const [downloading, setDownloading] = useState(false);
  const [viewerOpen, setViewerOpen] = useState(false);

  // 下载文件：优先用后端预签名地址，本地乐观消息则用内存里的 base64
  const handleDownload = async () => {
    if (!fileMessage || downloading) return;
    setDownloading(true);
    try {
      let url = "";
      if (fileMessage.fileId) {
        const rsp = await getSingleFile(fileMessage.fileId);
        url = rsp.success === true ? (rsp.fileData?.presignedUrl ?? "") : "";
      }
      if (!url && fileMessage.fileContent) {
        const binary = atob(fileMessage.fileContent);
        const bytes = new Uint8Array(binary.length);
        for (let i = 0; i < binary.length; i += 1) {
          bytes[i] = binary.charCodeAt(i);
        }
        url = URL.createObjectURL(new Blob([bytes]));
      }
      if (!url) return;

      const link = document.createElement("a");
      link.href = url;
      link.download = fileMessage.fileName ?? "文件";
      link.rel = "noopener";
      document.body.appendChild(link);
      link.click();
      link.remove();
    } catch {
      // 预签名地址获取失败时保持静默，用户可再次点击重试
    } finally {
      setDownloading(false);
    }
  };

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
          <button
            type="button"
            data-user-avatar
            aria-label={`查看 ${nickname} 的资料`}
            onClick={(e) => {
              const rect = e.currentTarget.getBoundingClientRect();
              setInfoAnchor((prev) =>
                prev?.userId === msg.sender.userId
                  ? null
                  : {
                      userId: msg.sender.userId,
                      rect,
                      initial: {
                        userId: msg.sender.userId,
                        nickname,
                        avatar: avatarFileId,
                      },
                    },
              );
            }}
            className="block cursor-pointer rounded-full"
          >
            <Avatar fileId={avatarFileId} name={nickname} />
          </button>
        </div>
      )}
      {/* 发送失败：红叹号放在气泡左侧；原因文案由消息区末尾居中展示（AGENTS.md §6.6） */}
      {isSelf && msg.status === "failed" && (
        <button
          type="button"
          aria-label="发送失败，点击重发"
          onClick={() => onRetry?.(msg)}
          className="mb-1 mr-2 self-end rounded-base p-0.5"
          style={{ color: "var(--color-danger)" }}
        >
          <AlertCircle size={16} strokeWidth={1.5} />
        </button>
      )}
      <div className="flex max-w-[60%] flex-col">
        {isGroup && !isSelf && (
          <span className="mb-1 text-xs text-secondary">{nickname}</span>
        )}
        <div
          className={cn(
            "rounded-base",
            isImage || isFile
              ? "overflow-hidden"
              : "whitespace-pre-wrap break-words px-3 py-2.5 text-base",
            isFile
              ? "bg-input shadow-sm"
              : isSelf
                ? "bg-bubble-self"
                : "bg-bubble-other shadow-sm",
          )}
          style={{ color: "var(--bubble-text)" }}
        >
          {isImage ? (
            imageSrc && !imageBroken ? (
              <button
                type="button"
                aria-label="查看大图"
                onClick={() => setViewerOpen(true)}
                className="block cursor-zoom-in"
              >
                <img
                  src={imageSrc}
                  alt={fileMessage?.fileName ?? "图片"}
                  className="block max-h-60 max-w-60 object-cover"
                  loading="lazy"
                  onError={() => setImageBroken(true)}
                />
              </button>
            ) : (
              <div className="flex h-36 w-48 items-center justify-center text-tertiary">
                <ImageIcon size={28} strokeWidth={1.5} />
              </div>
            )
          ) : isFile ? (
            <button
              type="button"
              onClick={() => void handleDownload()}
              disabled={downloading}
              aria-label={`下载 ${fileMessage?.fileName ?? "文件"}`}
              className="flex w-[220px] items-center gap-3 px-3 py-2.5 text-left disabled:opacity-70"
            >
              <div className="min-w-0 flex-1">
                <p
                  className="truncate text-sm text-primary"
                  title={fileMessage?.fileName}
                >
                  {fileMessage?.fileName ?? "文件"}
                </p>
                {fileSizeText && (
                  <p className="mt-1 text-xs text-secondary">{fileSizeText}</p>
                )}
              </div>
              {/* 文件类型图标统一，放大并保留线条细节 */}
              <span className="flex shrink-0 items-center justify-center text-brand">
                <FileText size={30} strokeWidth={1.5} />
              </span>
            </button>
          ) : (
            content
          )}
        </div>
      </div>
      {/* 发送状态（AGENTS.md §6.6）：发送中提示 */}
      {isSelf && msg.status === "sending" && (
        <span className="mb-1 ml-2 self-end pb-1 text-xs text-tertiary">
          发送中
        </span>
      )}
      {isSelf && myId && (
        <div className="ml-3 mt-1">
          <button
            type="button"
            data-user-avatar
            aria-label="查看我的资料"
            onClick={(e) => {
              const rect = e.currentTarget.getBoundingClientRect();
              setInfoAnchor((prev) =>
                prev?.userId === myId
                  ? null
                  : {
                      userId: myId,
                      rect,
                      initial: { userId: myId, nickname, avatar: myAvatar },
                    },
              );
            }}
            className="block cursor-pointer rounded-full"
          >
            <Avatar fileId={myAvatar} name={nickname} />
          </button>
        </div>
      )}
      {infoAnchor && (
        <UserInfoPopup
          userId={infoAnchor.userId}
          anchor={infoAnchor.rect}
          initial={infoAnchor.initial}
          onClose={() => setInfoAnchor(null)}
        />
      )}
      {viewerOpen && (
        <ImageViewerDialog
          sessionId={msg.chatSessionId}
          startMessageId={msg.messageId}
          onClose={() => setViewerOpen(false)}
        />
      )}
    </div>
  );
}
