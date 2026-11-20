import { useEffect, useMemo, useState, type ReactNode } from "react";
import { ChevronRight, Loader2 } from "lucide-react";
import {
  getFriendList,
  getPendingFriendEvents,
  processFriendEvent,
} from "@/api/friend";
import { useFriendStore } from "@/stores/friend";
import { useSessionStore } from "@/stores/session";
import { useUiStore } from "@/stores/ui";
import { Avatar } from "@/components/Avatar";
import { FriendSearch } from "@/components/FriendSearch";
import { cn } from "@/components/ui/cn";
import { groupByInitial } from "@/lib/pinyin";
import type { ChatSessionInfo, FriendEvent, UserInfo } from "@/types";

// 可折叠分组（群聊 / 联系人）：标题行 + 数量，展开后按拼音首字母分组
function Section({
  title,
  count,
  open,
  onToggle,
  children,
}: {
  title: string;
  count: number;
  open: boolean;
  onToggle: () => void;
  children: ReactNode;
}) {
  return (
    <section>
      <button
        type="button"
        aria-expanded={open}
        onClick={onToggle}
        className="hv-list flex h-12 w-full items-center gap-2 px-3 text-left"
      >
        <ChevronRight
          size={16}
          strokeWidth={1.5}
          className={cn(
            "shrink-0 text-secondary transition-transform",
            open && "rotate-90",
          )}
        />
        <span className="flex-1 truncate text-sm text-primary">{title}</span>
        <span className="shrink-0 text-xs text-tertiary tabular-nums">
          {count}
        </span>
      </button>
      {open && <div className="pb-2">{children}</div>}
    </section>
  );
}

// 拼音分组标题 + 成员行（行高 56px，40px 圆角方形头像）
function GroupedList<T>({
  items,
  getName,
  getKey,
  renderRow,
}: {
  items: T[];
  getName: (item: T) => string;
  getKey: (item: T) => string;
  renderRow: (item: T) => ReactNode;
}) {
  const groups = useMemo(
    () => groupByInitial(items, getName),
    [items, getName],
  );
  return (
    <>
      {groups.map((group) => (
        <div key={group.letter}>
          <div className="px-3 pb-1 pt-2 text-xs text-tertiary">
            {group.letter}
          </div>
          <ul>
            {group.items.map((item) => (
              <li key={getKey(item)}>{renderRow(item)}</li>
            ))}
          </ul>
        </div>
      ))}
    </>
  );
}

function EmptyHint({ children }: { children: string }) {
  return (
    <div className="px-3 py-4 text-center text-xs text-tertiary">
      {children}
    </div>
  );
}

// 通讯录：群聊 / 联系人两个可折叠分组，均按拼音首字母排序
export function ContactsView() {
  const sessions = useSessionStore((s) => s.sessions);
  const setActive = useSessionStore((s) => s.setActive);
  const setSideView = useUiStore((s) => s.setSideView);
  const selectedContact = useUiStore((s) => s.selectedContact);
  const setSelectedContact = useUiStore((s) => s.setSelectedContact);

  const [groupsOpen, setGroupsOpen] = useState(true);
  const [contactsOpen, setContactsOpen] = useState(true);
  const [requestsOpen, setRequestsOpen] = useState(true);
  const [friends, setFriends] = useState<UserInfo[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [reloadKey, setReloadKey] = useState(0);
  const [processingId, setProcessingId] = useState<string | null>(null);
  const [applyError, setApplyError] = useState<string | null>(null);

  const pendingApplies = useFriendStore((s) => s.pendingApplies);
  const setPendingApplies = useFriendStore((s) => s.setPendingApplies);
  const removePendingApply = useFriendStore((s) => s.removePendingApply);
  const refreshSessions = useSessionStore((s) => s.refresh);

  const groups = useMemo(
    () => sessions.filter((s) => !s.singleChatFriendId),
    [sessions],
  );

  // 待处理好友申请全量拉取（WS 推送会实时补进来）
  useEffect(() => {
    let cancelled = false;
    getPendingFriendEvents()
      .then((rsp) => {
        if (cancelled || rsp.success !== true) return;
        setPendingApplies(rsp.event ?? []);
      })
      .catch(() => {
        // 拉取失败时保留 WS 已推送的申请
      });
    return () => {
      cancelled = true;
    };
  }, [setPendingApplies]);

  // 好友列表（GET /friend/list）
  useEffect(() => {
    let cancelled = false;
    getFriendList()
      .then((rsp) => {
        if (cancelled) return;
        if (rsp.success !== true) {
          setError(rsp.errmsg || "加载好友列表失败");
        } else {
          setFriends(rsp.friendList ?? []);
          setError(null);
        }
        setLoading(false);
      })
      .catch((err: unknown) => {
        if (cancelled) return;
        setError(err instanceof Error ? err.message : "加载好友列表失败");
        setLoading(false);
      });
    return () => {
      cancelled = true;
    };
  }, [reloadKey]);

  // 联系人 → 单聊会话（用于进入聊天；资料页里的「发消息」会按需创建会话）
  const sessionByFriendId = useMemo(() => {
    const map = new Map<string, ChatSessionInfo>();
    sessions.forEach((s) => {
      if (s.singleChatFriendId) map.set(s.singleChatFriendId, s);
    });
    return map;
  }, [sessions]);

  const openSession = (sessionId: string) => {
    setActive(sessionId);
    setSelectedContact(null);
    setSideView("chats");
  };

  const groupName = (s: ChatSessionInfo) => s.chatSessionName || "未命名会话";

  // 同意/拒绝好友申请：成功后移除条目；同意时刷新会话与联系人
  const handleProcess = async (event: FriendEvent, agree: boolean) => {
    if (processingId) return;
    setProcessingId(event.eventId);
    setApplyError(null);
    try {
      const rsp = await processFriendEvent(
        event.eventId,
        agree,
        event.sender?.userId ?? "",
      );
      if (rsp.success !== true) {
        setApplyError(rsp.errmsg || "处理好友申请失败");
        return;
      }
      removePendingApply(event.eventId);
      if (agree) {
        await refreshSessions();
        setReloadKey((k) => k + 1);
      }
    } catch (err: unknown) {
      setApplyError(err instanceof Error ? err.message : "处理好友申请失败");
    } finally {
      setProcessingId(null);
    }
  };

  return (
    <div className="flex h-full flex-col">
      <FriendSearch />
      <div className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto">
        {pendingApplies.length > 0 && (
          <Section
            title="新的朋友"
            count={pendingApplies.length}
            open={requestsOpen}
            onToggle={() => setRequestsOpen((open) => !open)}
          >
            {applyError && (
              <div className="px-3 pb-1 pt-1 text-xs text-danger">
                {applyError}
              </div>
            )}
            {pendingApplies.map((event) => {
              const sender = event.sender;
              const name = sender?.nickname || "未命名";
              const busy = processingId === event.eventId;
              return (
                <div
                  key={event.eventId}
                  className="flex h-14 items-center gap-3 px-3"
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
            })}
          </Section>
        )}

        <Section
          title="群聊"
          count={groups.length}
          open={groupsOpen}
          onToggle={() => setGroupsOpen((open) => !open)}
        >
          {groups.length === 0 ? (
            <EmptyHint>暂无群聊</EmptyHint>
          ) : (
            <GroupedList
              items={groups}
              getName={groupName}
              getKey={(s) => s.chatSessionId}
              renderRow={(session) => (
                <button
                  type="button"
                  onClick={() => openSession(session.chatSessionId)}
                  className="hv-list flex h-14 w-full items-center gap-3 px-3 text-left"
                >
                  <Avatar
                    fileId={session.avatar}
                    name={groupName(session)}
                    size={40}
                  />
                  <span className="min-w-0 flex-1 truncate text-sm text-primary">
                    {groupName(session)}
                  </span>
                </button>
              )}
            />
          )}
        </Section>

        <Section
          title="联系人"
          count={friends.length}
          open={contactsOpen}
          onToggle={() => setContactsOpen((open) => !open)}
        >
          {loading ? (
            <div className="flex items-center justify-center gap-2 px-3 py-4 text-xs text-tertiary">
              <Loader2 size={14} className="animate-spin" aria-hidden="true" />
              加载中…
            </div>
          ) : error ? (
            <div className="flex items-center justify-center gap-3 px-3 py-4 text-xs text-secondary">
              <span>{error}</span>
              <button
                type="button"
                onClick={() => {
                  setLoading(true);
                  setError(null);
                  setReloadKey((k) => k + 1);
                }}
                className="text-brand"
              >
                重试
              </button>
            </div>
          ) : friends.length === 0 ? (
            <EmptyHint>暂无好友</EmptyHint>
          ) : (
            <GroupedList
              items={friends}
              getName={(user) => user.nickname || "未命名"}
              getKey={(user) => user.userId}
              renderRow={(user) => {
                const name = user.nickname || "未命名";
                const hasSession = sessionByFriendId.has(user.userId);
                const selected = selectedContact?.userId === user.userId;
                return (
                  <button
                    type="button"
                    aria-current={selected ? "true" : undefined}
                    onClick={() => setSelectedContact(user)}
                    className={cn(
                      "flex h-14 w-full items-center gap-3 px-3 text-left",
                      selected
                        ? "bg-brand text-inverse"
                        : "hv-list text-primary",
                    )}
                    title={hasSession ? `与 ${name} 聊天` : name}
                  >
                    <Avatar fileId={user.avatar} name={name} size={40} />
                    <span
                      className={cn(
                        "min-w-0 flex-1 truncate text-sm",
                        selected ? "text-inverse" : "text-primary",
                      )}
                    >
                      {name}
                    </span>
                  </button>
                );
              }}
            />
          )}
        </Section>
      </div>
    </div>
  );
}
