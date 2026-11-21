import { useEffect, useMemo, useRef, useState } from "react";
import { createPortal } from "react-dom";
import { Check, Loader2, MinusCircle, Search } from "lucide-react";
import {
  createChatSession,
  getChatSessionList,
  getFriendList,
} from "@/api/friend";
import { useSessionStore } from "@/stores/session";
import { useUiStore } from "@/stores/ui";
import { Avatar } from "@/components/Avatar";
import { cn } from "@/components/ui/cn";
import { groupByInitial } from "@/lib/pinyin";
import type { UserInfo } from "@/types";

interface GroupCreateDialogProps {
  onClose: () => void;
}

const nameOf = (user: UserInfo) => user.nickname || "未命名";

// 发起群聊：左侧好友（拼音分组 + 勾选），右侧已选（拼音排序）与群名称
export function GroupCreateDialog({ onClose }: GroupCreateDialogProps) {
  const setSessions = useSessionStore((s) => s.setSessions);
  const setActive = useSessionStore((s) => s.setActive);
  const setSideView = useUiStore((s) => s.setSideView);

  const [friends, setFriends] = useState<UserInfo[]>([]);
  const [loading, setLoading] = useState(true);
  const [loadError, setLoadError] = useState<string | null>(null);
  const [keyword, setKeyword] = useState("");
  const [selectedIds, setSelectedIds] = useState<string[]>([]);
  const [groupName, setGroupName] = useState("");
  const [creating, setCreating] = useState(false);
  const [createError, setCreateError] = useState<string | null>(null);
  const searchRef = useRef<HTMLInputElement | null>(null);

  useEffect(() => {
    let cancelled = false;
    getFriendList()
      .then((rsp) => {
        if (cancelled) return;
        if (rsp.success !== true) {
          setLoadError(rsp.errmsg || "好友列表加载失败");
          return;
        }
        setFriends(rsp.friendList ?? []);
      })
      .catch((err: unknown) => {
        if (!cancelled) {
          setLoadError(err instanceof Error ? err.message : "好友列表加载失败");
        }
      })
      .finally(() => {
        if (!cancelled) setLoading(false);
      });
    return () => {
      cancelled = true;
    };
  }, []);

  useEffect(() => {
    searchRef.current?.focus();
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape" && !creating) onClose();
    };
    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  }, [onClose, creating]);

  const selected = useMemo(
    () => friends.filter((f) => selectedIds.includes(f.userId)),
    [friends, selectedIds],
  );
  // 右侧已选列表按拼音顺序展示（不显示分组标题）
  const selectedSorted = useMemo(
    () => groupByInitial(selected, nameOf).flatMap((group) => group.items),
    [selected],
  );

  const filtered = useMemo(() => {
    const key = keyword.trim().toLowerCase();
    if (!key) return friends;
    return friends.filter((f) => nameOf(f).toLowerCase().includes(key));
  }, [friends, keyword]);

  const groups = useMemo(() => groupByInitial(filtered, nameOf), [filtered]);

  const toggle = (userId: string) =>
    setSelectedIds((ids) =>
      ids.includes(userId)
        ? ids.filter((id) => id !== userId)
        : [...ids, userId],
    );

  const canCreate = selectedIds.length >= 2 && !creating;

  const handleCreate = async () => {
    if (!canCreate) return;
    setCreating(true);
    setCreateError(null);
    // 未填写名称时用成员昵称拼接：张三、李四、王五等
    const fallback =
      selectedSorted.map(nameOf).slice(0, 3).join("、") +
      (selectedSorted.length > 3 ? "等" : "");
    const name = groupName.trim() || fallback;
    try {
      const rsp = await createChatSession(name, selectedIds);
      if (rsp.success !== true || !rsp.chatSessionId) {
        setCreateError(rsp.errmsg || "创建群聊失败");
        return;
      }
      const listRsp = await getChatSessionList();
      if (listRsp.success === true) {
        setSessions(listRsp.chatSessionInfoList ?? []);
      }
      setActive(rsp.chatSessionId);
      setSideView("chats");
      onClose();
    } catch (err: unknown) {
      setCreateError(err instanceof Error ? err.message : "创建群聊失败");
    } finally {
      setCreating(false);
    }
  };

  return createPortal(
    <div className="fixed inset-0 z-50 flex items-center justify-center p-6">
      <div
        aria-hidden="true"
        className="absolute inset-0"
        style={{ background: "var(--overlay-bg)" }}
        onClick={() => {
          if (!creating) onClose();
        }}
      />
      <div
        role="dialog"
        aria-modal="true"
        aria-label="发起群聊"
        className="relative flex h-[460px] w-[640px] overflow-hidden rounded-xl border border-line bg-panel shadow-popup"
      >
        {/* 左侧：好友列表 */}
        <div className="flex w-[300px] flex-col border-r border-line">
          <div className="p-3">
            <div
              className="flex h-8 items-center rounded-base px-2"
              style={{ background: "var(--search-bg)" }}
            >
              <Search size={16} strokeWidth={1.5} className="text-tertiary" />
              <input
                ref={searchRef}
                value={keyword}
                onChange={(e) => setKeyword(e.target.value)}
                placeholder="搜索"
                aria-label="搜索好友"
                className="ml-2 h-full min-w-0 flex-1 bg-transparent text-sm text-primary outline-none placeholder:text-tertiary focus-visible:outline-none"
              />
            </div>
          </div>
          <div className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto pb-2">
            <p className="px-3 py-1 text-xs text-tertiary">联系人</p>
            {loading ? (
              <p className="px-3 py-6 text-center text-xs text-tertiary">
                加载中…
              </p>
            ) : loadError ? (
              <p className="px-3 py-6 text-center text-xs text-danger">
                {loadError}
              </p>
            ) : groups.length === 0 ? (
              <p className="px-3 py-6 text-center text-xs text-tertiary">
                未找到联系人
              </p>
            ) : (
              groups.map((group) => (
                <div key={group.letter}>
                  <p className="px-3 py-1 text-xs text-tertiary">
                    {group.letter}
                  </p>
                  {group.items.map((user) => {
                    const name = nameOf(user);
                    const checked = selectedIds.includes(user.userId);
                    return (
                      <button
                        key={user.userId}
                        type="button"
                        aria-pressed={checked}
                        onClick={() => toggle(user.userId)}
                        className="hv-list flex w-full items-center gap-3 px-3 py-2 text-left"
                      >
                        <span
                          className={cn(
                            "flex h-5 w-5 shrink-0 items-center justify-center rounded-full border",
                            checked
                              ? "border-brand bg-brand text-inverse"
                              : "border-line text-transparent",
                          )}
                        >
                          <Check size={12} strokeWidth={3} />
                        </span>
                        <Avatar fileId={user.avatar} name={name} size={36} />
                        <span className="min-w-0 flex-1 truncate text-sm text-primary">
                          {name}
                        </span>
                      </button>
                    );
                  })}
                </div>
              ))
            )}
          </div>
        </div>

        {/* 右侧：已选列表 + 群名称 + 操作 */}
        <div className="flex min-w-0 flex-1 flex-col">
          <div className="flex h-12 shrink-0 items-center justify-between px-4">
            <span className="text-sm text-primary">发起群聊</span>
            <span className="text-xs text-tertiary">
              已选择 {selectedIds.length} 个联系人
            </span>
          </div>
          <div className="shrink-0 px-4 pb-2">
            <input
              value={groupName}
              onChange={(e) => setGroupName(e.target.value)}
              placeholder="群聊名称（选填，默认取成员昵称）"
              aria-label="群聊名称"
              maxLength={40}
              className="h-8 w-full rounded-base bg-input px-2 text-sm text-primary outline-none placeholder:text-tertiary focus-visible:outline-none"
            />
          </div>
          <div className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto">
            {selectedSorted.map((user) => {
              const name = nameOf(user);
              return (
                <div
                  key={user.userId}
                  className="flex items-center gap-3 px-4 py-2"
                >
                  <Avatar fileId={user.avatar} name={name} size={36} />
                  <span className="min-w-0 flex-1 truncate text-sm text-primary">
                    {name}
                  </span>
                  <button
                    type="button"
                    aria-label={`移除 ${name}`}
                    onClick={() => toggle(user.userId)}
                    className="shrink-0 text-tertiary transition-colors hover:text-secondary"
                  >
                    <MinusCircle size={18} strokeWidth={1.5} />
                  </button>
                </div>
              );
            })}
          </div>
          {createError && (
            <p className="shrink-0 px-4 pb-1 text-xs text-danger">
              {createError}
            </p>
          )}
          <div className="flex shrink-0 justify-end gap-3 px-4 py-3">
            <button
              type="button"
              disabled={!canCreate}
              onClick={() => void handleCreate()}
              className="flex h-9 min-w-20 items-center justify-center gap-2 rounded-base bg-brand px-4 text-sm text-inverse disabled:opacity-60"
            >
              {creating && (
                <Loader2
                  size={14}
                  className="animate-spin"
                  aria-hidden="true"
                />
              )}
              完成
            </button>
            <button
              type="button"
              disabled={creating}
              onClick={onClose}
              className="h-9 rounded-base bg-input px-4 text-sm text-secondary disabled:opacity-60"
            >
              取消
            </button>
          </div>
        </div>
      </div>
    </div>,
    document.body,
  );
}
