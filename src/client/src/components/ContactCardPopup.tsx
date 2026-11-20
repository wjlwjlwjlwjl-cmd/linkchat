import { useCallback, useEffect, useRef, useState } from "react";
import { createPortal } from "react-dom";
import { Loader2 } from "lucide-react";
import {
  addFriend,
  getChatSessionList,
  getFriendList,
  getSingleChatSession,
} from "@/api/friend";
import { useSessionStore } from "@/stores/session";
import { useUiStore } from "@/stores/ui";
import { Avatar } from "@/components/Avatar";
import type { UserInfo } from "@/types";

const CARD_WIDTH = 300;
const GAP = 8;

interface ContactCardPopupProps {
  user: UserInfo;
  anchor: DOMRect;
  onClose: () => void;
}

// 搜索结果联系人卡片：资料 + 「添加到通讯录 / 发消息」
export function ContactCardPopup({
  user,
  anchor,
  onClose,
}: ContactCardPopupProps) {
  const setSessions = useSessionStore((s) => s.setSessions);
  const setActive = useSessionStore((s) => s.setActive);
  const setSideView = useUiStore((s) => s.setSideView);

  // null 表示好友关系还在确认中
  const [isFriend, setIsFriend] = useState<boolean | null>(null);
  const [sending, setSending] = useState(false);
  const [sent, setSent] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [top, setTop] = useState(anchor.top);
  const cardRef = useRef<HTMLDivElement | null>(null);

  const name = user.nickname || "未命名";

  // 已是好友时按钮改为「发消息」，需要用好友列表确认关系
  useEffect(() => {
    let cancelled = false;
    getFriendList()
      .then((rsp) => {
        if (cancelled) return;
        const friends = rsp.success === true ? (rsp.friendList ?? []) : [];
        setIsFriend(friends.some((f) => f.userId === user.userId));
      })
      .catch(() => {
        if (!cancelled) setIsFriend(false);
      });
    return () => {
      cancelled = true;
    };
  }, [user.userId]);

  // 垂直方向按卡片实际高度收敛到视口内
  const measure = useCallback(
    (el: HTMLDivElement | null) => {
      cardRef.current = el;
      if (!el) return;
      const maxTop = window.innerHeight - el.offsetHeight - GAP;
      setTop(Math.max(GAP, Math.min(anchor.top, maxTop)));
    },
    [anchor.top],
  );

  useEffect(() => {
    const onPointerDown = (e: MouseEvent) => {
      const target = e.target as HTMLElement;
      if (cardRef.current?.contains(target)) return;
      onClose();
    };
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") onClose();
    };
    document.addEventListener("mousedown", onPointerDown);
    document.addEventListener("keydown", onKeyDown);
    return () => {
      document.removeEventListener("mousedown", onPointerDown);
      document.removeEventListener("keydown", onKeyDown);
    };
  }, [onClose]);

  // 已是好友：获取/创建单聊会话后进入消息视图
  const openChat = async () => {
    setSending(true);
    setError(null);
    try {
      const rsp = await getSingleChatSession(user.userId);
      const sessionId = rsp.chatSessionId;
      if (rsp.success !== true || !sessionId) {
        setError(rsp.errmsg || "打开会话失败");
        return;
      }
      const listRsp = await getChatSessionList();
      if (listRsp.success === true) {
        setSessions(listRsp.chatSessionInfoList ?? []);
      }
      setActive(sessionId);
      setSideView("chats");
      onClose();
    } catch (err: unknown) {
      setError(err instanceof Error ? err.message : "打开会话失败");
    } finally {
      setSending(false);
    }
  };

  const sendRequest = async () => {
    setSending(true);
    setError(null);
    try {
      const rsp = await addFriend(user.userId);
      if (rsp.success !== true) {
        setError(rsp.errmsg || "发送好友申请失败");
        return;
      }
      setSent(true);
    } catch (err: unknown) {
      setError(err instanceof Error ? err.message : "发送好友申请失败");
    } finally {
      setSending(false);
    }
  };

  const handlePrimary = () => {
    if (sending || sent || isFriend === null) return;
    void (isFriend ? openChat() : sendRequest());
  };

  const buttonText = sent
    ? "已发送申请"
    : sending
      ? isFriend
        ? "正在打开…"
        : "发送中…"
      : isFriend
        ? "发消息"
        : "添加到通讯录";

  const left =
    anchor.right + GAP + CARD_WIDTH <= window.innerWidth - GAP
      ? anchor.right + GAP
      : Math.max(GAP, anchor.left - CARD_WIDTH - GAP);

  return createPortal(
    <div
      ref={measure}
      data-contact-card
      role="dialog"
      aria-label={`${name} 的联系人卡片`}
      className="fixed z-50 overflow-y-auto rounded-xl border border-line bg-panel p-5 shadow-popup"
      style={{
        left,
        top,
        width: CARD_WIDTH,
        maxHeight: `calc(100vh - ${2 * GAP}px)`,
      }}
    >
      <div className="flex items-start gap-4">
        <Avatar fileId={user.avatar} name={name} size={64} />
        <div className="min-w-0 flex-1 pt-1">
          <p className="truncate text-base font-medium text-primary">{name}</p>
          <p className="mt-1.5 truncate text-xs text-tertiary">
            {user.email || "未填写邮箱"}
          </p>
        </div>
      </div>

      <div className="my-4 h-px bg-line" />

      <div className="flex gap-2 text-xs leading-5">
        <span className="shrink-0 text-tertiary">签名：</span>
        <span className="min-w-0 break-all text-secondary">
          {user.description || "这个人很懒，什么都没写"}
        </span>
      </div>

      <div className="mt-5">
        <button
          type="button"
          onClick={handlePrimary}
          disabled={sending || sent || isFriend === null}
          className="flex h-9 w-full items-center justify-center gap-2 rounded-base bg-brand text-sm text-inverse disabled:opacity-60"
        >
          {sending && (
            <Loader2 size={14} className="animate-spin" aria-hidden="true" />
          )}
          {buttonText}
        </button>
        {error && (
          <p className="mt-2 text-center text-xs text-danger">{error}</p>
        )}
      </div>
    </div>,
    document.body,
  );
}
