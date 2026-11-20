import { useEffect, useState } from "react";
import { createPortal } from "react-dom";
import dayjs from "dayjs";
import { Image as ImageIcon, Search, X } from "lucide-react";
import { searchMessages } from "@/api/message";
import { useAuthStore } from "@/stores/auth";
import { useMessageStore } from "@/stores/message";
import { useUserStore } from "@/stores/user";
import { Avatar } from "@/components/Avatar";
import { useAvatarUrl } from "@/hooks/useAvatarUrl";
import { imageMimeType, isImageFileName } from "@/lib/file";
import type { ChatSessionInfo, MessageInfo } from "@/types";

interface MessageSearchDialogProps {
  session: ChatSessionInfo;
  onClose: () => void;
}

// 新的在前（搜索结果与默认记录都按这个顺序展示）
function byTimeDesc(a: MessageInfo, b: MessageInfo): number {
  const diff = Number(b.createTime) - Number(a.createTime);
  return diff !== 0 ? diff : b.messageId.localeCompare(a.messageId);
}

// 单条搜索结果：头像 + 昵称 + 时间 + 文本 / 图片缩略图
function SearchResultItem({ msg }: { msg: MessageInfo }) {
  const myId = useAuthStore((s) => s.profile?.userId);
  const myName = useAuthStore((s) => s.profile?.nickname);
  const myAvatar = useAuthStore((s) => s.profile?.avatar);
  const otherUser = useUserStore((s) => s.users[msg.sender.userId]);

  const isSelf = myId !== undefined && msg.sender.userId === myId;
  const nickname = isSelf
    ? (myName ?? "我")
    : (otherUser?.nickname ?? msg.sender.nickname ?? "未知用户");
  const avatarFileId = isSelf
    ? myAvatar
    : (otherUser?.avatar ?? msg.sender.avatar);

  const fileMessage = msg.message.fileMessage;
  const isImage =
    fileMessage !== undefined && isImageFileName(fileMessage.fileName);
  const imageContent = fileMessage?.fileContent ?? "";
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
  const time = dayjs.unix(Number(msg.createTime)).format("YYYY年M月D日 HH:mm");

  return (
    <div className="flex gap-3 border-b border-line-light py-3 last:border-b-0">
      <Avatar fileId={avatarFileId} name={nickname} size={36} />
      <div className="min-w-0 flex-1">
        <div className="flex items-baseline justify-between gap-3">
          <span className="truncate text-sm text-secondary">{nickname}</span>
          <span className="shrink-0 text-xs text-tertiary tabular-nums">
            {time}
          </span>
        </div>
        {isImage ? (
          imageSrc && !imageBroken ? (
            <img
              src={imageSrc}
              alt={fileMessage?.fileName ?? "图片"}
              className="mt-2 block max-h-40 max-w-40 rounded-base object-cover"
              loading="lazy"
              onError={() => setImageBroken(true)}
            />
          ) : (
            <div className="mt-2 flex h-24 w-32 items-center justify-center rounded-base bg-input text-tertiary">
              <ImageIcon size={24} strokeWidth={1.5} />
            </div>
          )
        ) : (
          <p className="mt-1 line-clamp-3 whitespace-pre-wrap break-words text-sm text-primary">
            {content}
          </p>
        )}
      </div>
    </div>
  );
}

// 聊天记录搜索弹窗：默认展示打开时当前会话的消息快照（不随 WS 更新），回车走 /message/search
export function MessageSearchDialog({
  session,
  onClose,
}: MessageSearchDialogProps) {
  // 只在挂载时取一次快照，保证默认列表不随后续推送变化
  const [snapshot] = useState<MessageInfo[]>(() =>
    [
      ...(useMessageStore.getState().messages[session.chatSessionId] ?? []),
    ].sort(byTimeDesc),
  );
  const [keyword, setKeyword] = useState("");
  const [results, setResults] = useState<MessageInfo[] | null>(null);
  const [searching, setSearching] = useState(false);
  const [error, setError] = useState("");

  useEffect(() => {
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") onClose();
    };
    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  }, [onClose]);

  const submit = async () => {
    const key = keyword.trim();
    if (!key) {
      setResults(null);
      setError("");
      return;
    }
    setSearching(true);
    setError("");
    try {
      const rsp = await searchMessages({
        chatSessionId: session.chatSessionId,
        searchKey: key,
      });
      if (rsp.success !== true) {
        setError(rsp.errmsg || "搜索失败");
        setResults([]);
        return;
      }
      setResults([...(rsp.msgList ?? [])].sort(byTimeDesc));
    } catch (err) {
      setError(err instanceof Error ? err.message : "搜索失败");
      setResults([]);
    } finally {
      setSearching(false);
    }
  };

  const list = results ?? snapshot;

  return createPortal(
    <div
      className="fixed inset-0 z-50 flex items-center justify-center"
      style={{ backgroundColor: "var(--overlay-bg)" }}
      onMouseDown={(e) => {
        if (e.target === e.currentTarget) onClose();
      }}
    >
      <div
        role="dialog"
        aria-modal="true"
        aria-label="搜索聊天记录"
        className="flex h-[70vh] max-h-[640px] w-[720px] max-w-[calc(100vw-48px)] flex-col overflow-hidden rounded-lg border border-line bg-panel shadow-popup"
      >
        <header className="flex h-header shrink-0 items-center justify-between border-b border-line px-5">
          <h2 className="text-base font-medium text-primary">
            “{session.chatSessionName}”的聊天记录
            {list.length > 0 ? `(${list.length})` : ""}
          </h2>
          <button
            type="button"
            aria-label="关闭"
            onClick={onClose}
            className="hv-icon rounded-base p-1.5 text-secondary"
          >
            <X size={18} strokeWidth={1.5} />
          </button>
        </header>

        <div className="shrink-0 px-5 pt-4">
          <div className="flex items-center gap-2 rounded-pill border border-line bg-input px-3 py-2">
            <Search size={16} strokeWidth={1.5} className="text-tertiary" />
            <input
              autoFocus
              type="text"
              value={keyword}
              onChange={(e) => setKeyword(e.target.value)}
              onKeyDown={(e) => {
                if (e.key === "Enter") {
                  e.preventDefault();
                  void submit();
                }
              }}
              placeholder="搜索"
              aria-label="搜索聊天记录"
              className="w-full bg-transparent text-sm text-primary placeholder:text-tertiary focus-visible:outline-none"
            />
          </div>
          {error && (
            <p role="alert" className="mt-2 text-sm text-danger">
              {error}
            </p>
          )}
        </div>

        <div className="scroll-area mt-2 min-h-0 flex-1 overflow-y-auto px-5 pb-4">
          {searching ? (
            <p className="py-8 text-center text-sm text-secondary">搜索中…</p>
          ) : list.length === 0 ? (
            <p className="py-8 text-center text-sm text-secondary">
              {results === null ? "暂无聊天记录" : "未找到相关聊天记录"}
            </p>
          ) : (
            list.map((msg) => (
              <SearchResultItem key={msg.messageId} msg={msg} />
            ))
          )}
        </div>
      </div>
    </div>,
    document.body,
  );
}
