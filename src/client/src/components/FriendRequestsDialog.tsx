import { useEffect, useRef, useState } from "react";
import { createPortal } from "react-dom";
import { X } from "lucide-react";
import { processFriendEvent } from "@/api/friend";
import { useFriendStore } from "@/stores/friend";
import { useSessionStore } from "@/stores/session";
import { useUiStore } from "@/stores/ui";
import { Avatar } from "@/components/Avatar";
import type { FriendEvent } from "@/types";

// 好友申请独立弹窗：从 Rail 图标打开，逐条展示申请人并支持同意/拒绝
export function FriendRequestsDialog() {
  const open = useUiStore((s) => s.friendRequestsOpen);
  const setOpen = useUiStore((s) => s.setFriendRequestsOpen);
  const pendingApplies = useFriendStore((s) => s.pendingApplies);
  const removePendingApply = useFriendStore((s) => s.removePendingApply);
  const bumpFriendListVersion = useFriendStore((s) => s.bumpFriendListVersion);
  const refreshSessions = useSessionStore((s) => s.refresh);

  const [processingId, setProcessingId] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);
  const closeRef = useRef<HTMLButtonElement | null>(null);

  useEffect(() => {
    if (!open) return;
    closeRef.current?.focus();
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") setOpen(false);
    };
    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  }, [open, setOpen]);

  if (!open) return null;

  // 同意/拒绝：成功后移除条目；同意时刷新会话列表与通讯录好友
  const handleProcess = async (event: FriendEvent, agree: boolean) => {
    if (processingId) return;
    setProcessingId(event.eventId);
    setError(null);
    try {
      const rsp = await processFriendEvent(
        event.eventId,
        agree,
        event.sender?.userId ?? "",
      );
      if (rsp.success !== true) {
        setError(rsp.errmsg || "处理好友申请失败");
        return;
      }
      removePendingApply(event.eventId);
      if (agree) {
        await refreshSessions();
        bumpFriendListVersion();
      }
    } catch (err: unknown) {
      setError(err instanceof Error ? err.message : "处理好友申请失败");
    } finally {
      setProcessingId(null);
    }
  };

  return createPortal(
    <div className="fixed inset-0 z-50 flex items-center justify-center p-6">
      <div
        aria-hidden="true"
        className="absolute inset-0"
        style={{ background: "var(--overlay-bg)" }}
        onClick={() => setOpen(false)}
      />
      <div
        role="dialog"
        aria-modal="true"
        aria-label="新的朋友"
        className="relative flex max-h-[70vh] w-[360px] flex-col overflow-hidden rounded-xl border border-line bg-panel shadow-popup"
      >
        <div className="flex h-12 shrink-0 items-center justify-between border-b border-line px-4">
          <h2 className="text-sm text-primary">
            新的朋友
            {pendingApplies.length > 0 && (
              <span className="ml-1.5 text-xs text-tertiary">
                {pendingApplies.length}
              </span>
            )}
          </h2>
          <button
            ref={closeRef}
            type="button"
            aria-label="关闭"
            onClick={() => setOpen(false)}
            className="hv-icon rounded-base p-1 text-tertiary"
          >
            <X size={16} strokeWidth={1.5} />
          </button>
        </div>

        {error && (
          <div className="shrink-0 border-b border-danger-weak bg-danger-weak px-4 py-2 text-xs text-danger">
            {error}
          </div>
        )}

        <div className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto">
          {pendingApplies.length === 0 ? (
            <p className="py-10 text-center text-sm text-tertiary">
              暂无新的朋友申请
            </p>
          ) : (
            pendingApplies.map((event) => {
              const sender = event.sender;
              const name = sender?.nickname || "未命名";
              const busy = processingId === event.eventId;
              return (
                <div
                  key={event.eventId}
                  className="flex h-14 items-center gap-3 px-4"
                >
                  <Avatar fileId={sender?.avatar} name={name} size={40} />
                  <div className="min-w-0 flex-1">
                    <p className="truncate text-sm text-primary">{name}</p>
                    <p className="truncate text-xs text-tertiary">
                      请求添加你为好友
                    </p>
                  </div>
                  <button
                    type="button"
                    disabled={busy}
                    onClick={() => void handleProcess(event, true)}
                    className="shrink-0 rounded-base bg-brand px-2 py-1 text-xs text-inverse disabled:opacity-60"
                  >
                    同意
                  </button>
                  <button
                    type="button"
                    disabled={busy}
                    onClick={() => void handleProcess(event, false)}
                    className="hv-list shrink-0 rounded-base px-2 py-1 text-xs text-secondary disabled:opacity-60"
                  >
                    拒绝
                  </button>
                </div>
              );
            })
          )}
        </div>
      </div>
    </div>,
    document.body,
  );
}
