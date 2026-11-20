import { useCallback, useEffect, useRef, useState } from "react";
import { createPortal } from "react-dom";
import { getMultiUserInfo } from "@/api/user";
import { useAuthStore } from "@/stores/auth";
import { Avatar } from "@/components/Avatar";
import type { UserInfo } from "@/types";

const CARD_WIDTH = 288;
const GAP = 8;

interface UserInfoPopupProps {
  userId: string;
  anchor: DOMRect;
  // 已知资料（消息 sender / 群成员），用于首屏占位
  initial?: UserInfo;
  onClose: () => void;
}

// 资料行：标签 + 值
function InfoRow({ label, value }: { label: string; value: string }) {
  return (
    <div className="flex gap-1 text-xs leading-5">
      <span className="shrink-0 text-tertiary">{label}：</span>
      <span className="min-w-0 break-all text-secondary">{value}</span>
    </div>
  );
}

// 点击头像后弹出的资料卡：昵称 / 邮箱 / 签名
export function UserInfoPopup({
  userId,
  anchor,
  initial,
  onClose,
}: UserInfoPopupProps) {
  const profile = useAuthStore((s) => s.profile);
  const isSelf = profile?.userId === userId;
  const [info, setInfo] = useState<UserInfo | undefined>(
    isSelf ? profile : initial,
  );
  const [top, setTop] = useState(anchor.top);
  const cardRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    if (isSelf) return;
    let cancelled = false;
    getMultiUserInfo([userId])
      .then((rsp) => {
        if (cancelled) return;
        const next = rsp.usersInfo?.[userId];
        if (next) setInfo(next);
      })
      .catch(() => {
        // 失败时保留已知资料
      });
    return () => {
      cancelled = true;
    };
  }, [isSelf, userId]);

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
      // 头像自身交给点击逻辑切换，避免关掉又立刻打开
      if (target.closest("[data-user-avatar]")) return;
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

  const nickname = info?.nickname || "未命名";
  const left =
    anchor.right + GAP + CARD_WIDTH <= window.innerWidth - GAP
      ? anchor.right + GAP
      : Math.max(GAP, anchor.left - CARD_WIDTH - GAP);

  return createPortal(
    <div
      ref={measure}
      role="dialog"
      aria-label={`${nickname} 的资料`}
      className="fixed z-50 overflow-y-auto rounded-xl border border-line bg-panel p-4 shadow-popup"
      style={{
        left,
        top,
        width: CARD_WIDTH,
        maxHeight: `calc(100vh - ${2 * GAP}px)`,
      }}
    >
      <div className="flex items-start gap-3">
        <Avatar fileId={info?.avatar} name={nickname} size={56} />
        <div className="min-w-0 flex-1">
          <div className="truncate text-base font-medium text-primary">
            {nickname}
          </div>
          <div className="mt-2 space-y-0.5">
            <InfoRow label="昵称" value={nickname} />
            <InfoRow label="邮箱" value={info?.email || "未填写"} />
            <InfoRow
              label="签名"
              value={info?.description || "这个人很懒，什么都没写"}
            />
          </div>
        </div>
      </div>
    </div>,
    document.body,
  );
}
