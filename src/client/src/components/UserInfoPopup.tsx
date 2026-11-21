import { useCallback, useEffect, useRef, useState } from "react";
import { createPortal } from "react-dom";
import { MoreHorizontal } from "lucide-react";
import { getMultiUserInfo } from "@/api/user";
import { getFriendList, removeFriend } from "@/api/friend";
import { useAuthStore } from "@/stores/auth";
import { useFriendStore } from "@/stores/friend";
import { useSessionStore } from "@/stores/session";
import { useUiStore } from "@/stores/ui";
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

// 点击头像后弹出的资料卡：昵称 / 邮箱 / 签名 + 好友删除入口
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
  const [isFriend, setIsFriend] = useState(false);
  const [menuOpen, setMenuOpen] = useState(false);
  const [confirmOpen, setConfirmOpen] = useState(false);
  const [removing, setRemoving] = useState(false);
  const [deleteError, setDeleteError] = useState("");
  const [top, setTop] = useState(anchor.top);
  const cardRef = useRef<HTMLDivElement | null>(null);
  const menuRef = useRef<HTMLDivElement | null>(null);

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

  // 「删除好友」只对好友展示，需要先确认关系
  useEffect(() => {
    if (isSelf) return;
    let cancelled = false;
    getFriendList()
      .then((rsp) => {
        if (cancelled || rsp.success !== true) return;
        setIsFriend(
          (rsp.friendList ?? []).some((friend) => friend.userId === userId),
        );
      })
      .catch(() => {
        // 拉取失败时按非好友处理，不展示删除入口
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
      // 删除确认弹窗是独立遮罩，期间不关闭资料卡
      if (confirmOpen) return;
      if (menuOpen && menuRef.current && !menuRef.current.contains(target)) {
        setMenuOpen(false);
      }
      if (cardRef.current?.contains(target)) return;
      // 头像自身交给点击逻辑切换，避免关掉又立刻打开
      if (target.closest("[data-user-avatar]")) return;
      onClose();
    };
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key !== "Escape") return;
      if (removing) return;
      if (confirmOpen) {
        setConfirmOpen(false);
        setDeleteError("");
        return;
      }
      if (menuOpen) {
        setMenuOpen(false);
        return;
      }
      onClose();
    };
    document.addEventListener("mousedown", onPointerDown);
    document.addEventListener("keydown", onKeyDown);
    return () => {
      document.removeEventListener("mousedown", onPointerDown);
      document.removeEventListener("keydown", onKeyDown);
    };
  }, [confirmOpen, menuOpen, onClose, removing]);

  const handleRemoveFriend = async () => {
    if (removing) return;
    setRemoving(true);
    setDeleteError("");
    try {
      const rsp = await removeFriend(userId);
      if (rsp.success !== true) {
        setDeleteError(rsp.errmsg || "删除好友失败");
        return;
      }
      // 通讯录重新拉取好友列表；会话列表刷新后该单聊会消失
      useFriendStore.getState().bumpFriendListVersion();
      // 如果右侧资料页正展示该好友，一并关闭，避免继续显示「发消息」
      if (useUiStore.getState().selectedContact?.userId === userId) {
        useUiStore.getState().setSelectedContact(null);
      }
      void useSessionStore.getState().refresh();
      onClose();
    } catch (err) {
      setDeleteError(err instanceof Error ? err.message : "删除好友失败");
    } finally {
      setRemoving(false);
    }
  };

  const nickname = info?.nickname || "未命名";
  // 三个点常驻：好友显示「删除好友」，非好友显示「删除聊天」
  const canMenu = !isSelf;
  const menuLabel = isFriend ? "删除好友" : "删除聊天";
  const confirmTitle = menuLabel;
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
      {canMenu && (
        <div ref={menuRef} className="absolute right-2 top-2">
          <button
            type="button"
            aria-label="更多操作"
            aria-haspopup="menu"
            aria-expanded={menuOpen}
            onClick={() => setMenuOpen((open) => !open)}
            className="hv-icon rounded-base p-1 text-secondary"
          >
            <MoreHorizontal size={18} strokeWidth={1.5} />
          </button>
          {menuOpen && (
            <div
              role="menu"
              className="absolute right-0 top-full z-10 mt-1 w-28 overflow-hidden rounded-base border border-line bg-panel py-1 shadow-popup"
            >
              <button
                type="button"
                role="menuitem"
                onClick={() => {
                  setMenuOpen(false);
                  setDeleteError("");
                  setConfirmOpen(true);
                }}
                className="block w-full px-3 py-2 text-left text-sm text-danger"
              >
                {menuLabel}
              </button>
            </div>
          )}
        </div>
      )}

      <div className="flex items-start gap-3">
        <Avatar fileId={info?.avatar} name={nickname} size={56} />
        <div className="min-w-0 flex-1">
          <div
            className={`truncate text-base font-medium text-primary ${
              canMenu ? "pr-7" : ""
            }`}
          >
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

      {/* 删除好友二次确认 */}
      {confirmOpen &&
        createPortal(
          <div
            className="fixed inset-0 z-[60] flex items-center justify-center px-4"
            style={{ backgroundColor: "var(--overlay-bg)" }}
            onMouseDown={(e) => {
              if (e.target !== e.currentTarget || removing) return;
              setConfirmOpen(false);
              setDeleteError("");
            }}
          >
            <div
              role="dialog"
              aria-modal="true"
              aria-label={confirmTitle}
              className="w-[320px] rounded-lg border border-line bg-panel p-5 shadow-popup"
            >
              <h2 className="text-base font-medium text-primary">
                {confirmTitle}
              </h2>
              <p className="mt-3 text-sm text-secondary">
                {isFriend
                  ? `删除后将不再接收 ${nickname} 的消息，聊天会话也会从列表中移除。确定删除吗？`
                  : "删除后该聊天将从你的会话列表中移除。确定删除吗？"}
              </p>
              {deleteError && (
                <p role="alert" className="mt-3 text-sm text-danger">
                  {deleteError}
                </p>
              )}
              <div className="mt-5 flex justify-end gap-2">
                <button
                  type="button"
                  disabled={removing}
                  onClick={() => {
                    setConfirmOpen(false);
                    setDeleteError("");
                  }}
                  className="hv-icon h-8 rounded-base border border-line px-4 text-sm text-primary disabled:opacity-50"
                >
                  取消
                </button>
                <button
                  type="button"
                  disabled={removing}
                  onClick={() => void handleRemoveFriend()}
                  className="h-8 rounded-base bg-danger px-4 text-sm text-inverse disabled:opacity-50"
                >
                  {removing ? "删除中…" : "删除"}
                </button>
              </div>
            </div>
          </div>,
          document.body,
        )}
    </div>,
    document.body,
  );
}
