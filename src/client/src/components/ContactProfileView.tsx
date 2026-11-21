import { useEffect, useRef, useState } from "react";
import { createPortal } from "react-dom";
import { Loader2, MessageCircle, MoreHorizontal, UserPlus } from "lucide-react";
import {
  addFriend,
  getChatSessionList,
  getFriendApplyStatus,
  getFriendList,
  getSingleChatSession,
  removeFriend,
} from "@/api/friend";
import { useFriendStore } from "@/stores/friend";
import { useSessionStore } from "@/stores/session";
import { useUiStore } from "@/stores/ui";
import { Avatar } from "@/components/Avatar";
import type { UserInfo } from "@/types";

interface ContactProfileViewProps {
  contact: UserInfo;
}

// 资料行：左侧标签 + 右侧内容
function InfoRow({ label, value }: { label: string; value: string }) {
  return (
    <div className="flex gap-4 text-sm leading-6">
      <span className="w-16 shrink-0 text-secondary">{label}</span>
      <span className="min-w-0 break-all text-primary">{value}</span>
    </div>
  );
}

// 好友资料页：展示在聊天详情区，好友可发消息/删除，非好友可添加到通讯录
export function ContactProfileView({ contact }: ContactProfileViewProps) {
  const setSessions = useSessionStore((s) => s.setSessions);
  const setActive = useSessionStore((s) => s.setActive);
  const setSideView = useUiStore((s) => s.setSideView);
  const setSelectedContact = useUiStore((s) => s.setSelectedContact);
  // 同意申请/删除好友后自增，用于重新确认好友关系
  const friendListVersion = useFriendStore((s) => s.friendListVersion);

  // null 表示好友关系还在确认中
  const [isFriend, setIsFriend] = useState<boolean | null>(null);
  const [opening, setOpening] = useState(false);
  const [adding, setAdding] = useState(false);
  const [sent, setSent] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [menuOpen, setMenuOpen] = useState(false);
  const [confirmOpen, setConfirmOpen] = useState(false);
  const [removing, setRemoving] = useState(false);
  const [deleteError, setDeleteError] = useState("");
  const menuRef = useRef<HTMLDivElement | null>(null);

  const name = contact.nickname || "未命名";

  // 好友关系决定底部按钮是「发消息」还是「添加到通讯录」，以及是否展示删除入口
  useEffect(() => {
    let cancelled = false;
    getFriendList()
      .then((rsp) => {
        if (cancelled) return;
        const friends = rsp.success === true ? (rsp.friendList ?? []) : [];
        setIsFriend(friends.some((f) => f.userId === contact.userId));
      })
      .catch(() => {
        if (!cancelled) setIsFriend(false);
      });
    return () => {
      cancelled = true;
    };
  }, [contact.userId, friendListVersion]);

  // 非好友时确认是否已发过待处理申请，避免重复申请
  useEffect(() => {
    if (isFriend !== false || sent) return;
    let cancelled = false;
    getFriendApplyStatus(contact.userId)
      .then((rsp) => {
        if (!cancelled && rsp.success === true && rsp.pending === true) {
          setSent(true);
        }
      })
      .catch(() => {});
    return () => {
      cancelled = true;
    };
  }, [contact.userId, isFriend, sent]);

  // 点击菜单以外的区域时收起菜单
  useEffect(() => {
    if (!menuOpen) return;
    const onPointerDown = (e: MouseEvent) => {
      if (menuRef.current?.contains(e.target as Node)) return;
      setMenuOpen(false);
    };
    document.addEventListener("mousedown", onPointerDown);
    return () => document.removeEventListener("mousedown", onPointerDown);
  }, [menuOpen]);

  // 先确保单聊会话存在，再刷新会话列表并进入聊天
  const handleMessage = async () => {
    if (opening) return;
    setOpening(true);
    setError(null);
    try {
      const rsp = await getSingleChatSession(contact.userId);
      const sessionId = rsp.chatSessionId;
      if (rsp.success !== true || !sessionId) {
        setError(rsp.errmsg || "打开会话失败");
        return;
      }
      // 新建的会话不在本地缓存里，刷新列表后再选中
      const listRsp = await getChatSessionList();
      if (listRsp.success === true) {
        setSessions(listRsp.chatSessionInfoList ?? []);
      }
      setActive(sessionId);
      setSelectedContact(null);
      setSideView("chats");
    } catch (err: unknown) {
      setError(err instanceof Error ? err.message : "打开会话失败");
    } finally {
      setOpening(false);
    }
  };

  const handleAdd = async () => {
    if (adding || sent) return;
    setAdding(true);
    setError(null);
    try {
      const rsp = await addFriend(contact.userId);
      if (rsp.success !== true) {
        setError(rsp.errmsg || "发送好友申请失败");
        return;
      }
      setSent(true);
    } catch (err: unknown) {
      setError(err instanceof Error ? err.message : "发送好友申请失败");
    } finally {
      setAdding(false);
    }
  };

  const handleRemoveFriend = async () => {
    if (removing) return;
    setRemoving(true);
    setDeleteError("");
    try {
      const rsp = await removeFriend(contact.userId);
      if (rsp.success !== true) {
        setDeleteError(rsp.errmsg || "删除好友失败");
        return;
      }
      // 删除成功后：刷新好友列表与会话列表，并关闭当前资料页
      useFriendStore.getState().bumpFriendListVersion();
      void useSessionStore.getState().refresh();
      setSelectedContact(null);
    } catch (err) {
      setDeleteError(err instanceof Error ? err.message : "删除好友失败");
    } finally {
      setRemoving(false);
    }
  };

  const canDelete = isFriend === true;
  const primaryDisabled = opening || adding || sent || isFriend === null;
  const primaryText = sent
    ? "已发送申请"
    : isFriend
      ? opening
        ? "正在打开…"
        : "发消息"
      : adding
        ? "发送中…"
        : "添加到通讯录";

  return (
    <div className="relative flex h-full flex-col bg-app">
      {canDelete && (
        <div ref={menuRef} className="absolute right-6 top-6 z-10">
          <button
            type="button"
            aria-label="更多操作"
            aria-haspopup="menu"
            aria-expanded={menuOpen}
            onClick={() => setMenuOpen((open) => !open)}
            className="hv-icon rounded-base p-1 text-secondary"
          >
            <MoreHorizontal size={20} strokeWidth={1.5} />
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
                删除好友
              </button>
            </div>
          )}
        </div>
      )}

      <div className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto">
        <div className="mx-auto w-full max-w-[560px] px-8 pb-10 pt-16">
          <div className="flex items-start gap-5">
            <Avatar fileId={contact.avatar} name={name} size={80} />
            <div className="min-w-0 pt-1">
              <h2 className="truncate text-xl text-primary">{name}</h2>
              <p className="mt-2 truncate text-xs text-tertiary">
                {contact.email || "未填写邮箱"}
              </p>
            </div>
          </div>

          <section className="mt-10">
            <h3 className="text-xs text-tertiary">朋友资料</h3>
            <div className="mt-3 space-y-2">
              <InfoRow label="昵称" value={name} />
              <InfoRow
                label="签名"
                value={contact.description || "这个人很懒，什么都没写"}
              />
            </div>
          </section>

          <div className="my-8 h-px bg-line" />

          <section>
            <h3 className="text-xs text-tertiary">更多信息</h3>
            <div className="mt-3 space-y-2">
              <InfoRow label="邮箱" value={contact.email || "未填写"} />
              <InfoRow label="用户ID" value={contact.userId} />
            </div>
          </section>
        </div>
      </div>

      <div className="shrink-0 px-6 pb-10 pt-4">
        <div className="flex flex-col items-center gap-1.5">
          <button
            type="button"
            onClick={() => void (isFriend ? handleMessage() : handleAdd())}
            disabled={primaryDisabled}
            className="flex flex-col items-center gap-1.5 rounded-lg px-4 py-2 text-xs text-brand disabled:opacity-60"
          >
            {opening || adding ? (
              <Loader2 size={26} className="animate-spin" aria-hidden="true" />
            ) : isFriend ? (
              <MessageCircle size={26} strokeWidth={1.5} />
            ) : (
              <UserPlus size={26} strokeWidth={1.5} />
            )}
            <span>{primaryText}</span>
          </button>
          {error && <p className="text-xs text-danger">{error}</p>}
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
              aria-label="删除好友"
              className="w-[320px] rounded-lg border border-line bg-panel p-5 shadow-popup"
            >
              <h2 className="text-base font-medium text-primary">删除好友</h2>
              <p className="mt-3 text-sm text-secondary">
                删除后将不再接收 {name}{" "}
                的消息，聊天会话也会从列表中移除。确定删除吗？
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
    </div>
  );
}
