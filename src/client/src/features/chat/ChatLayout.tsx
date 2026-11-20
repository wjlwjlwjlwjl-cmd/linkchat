import { useCallback, useEffect, useRef, useState } from "react";
import { useSessionStore } from "@/stores/session";
import { useMessageStore } from "@/stores/message";
import { useUiStore } from "@/stores/ui";
import { useAuthStore } from "@/stores/auth";
import { useUserStore } from "@/stores/user";
import { getChatSessionList } from "@/api/friend";
import { getRecent } from "@/api/message";
import { getMultiUserInfo, getProfile } from "@/api/user";
import { Rail } from "@/components/Rail";
import { SessionList } from "@/components/SessionList";
import { MessageHeader } from "@/components/MessageHeader";
import { MessageList } from "@/components/MessageList";
import { InputArea } from "@/components/InputArea";
import { EmptyState } from "@/components/EmptyState";
import { ErrorState } from "@/components/ErrorState";
import type { MessageInfo } from "@/types";

// 按 createTime 升序（同时间按 messageId），后端 getRecent 返回倒序
function sortAsc(list: MessageInfo[]): MessageInfo[] {
  return [...list].sort((a, b) => {
    const ta = Number(a.createTime);
    const tb = Number(b.createTime);
    if (ta !== tb) return ta - tb;
    return a.messageId.localeCompare(b.messageId);
  });
}

// 三栏布局（style.md §2）：功能栏 60px + 会话列表 260px + 消息区
export function ChatLayout() {
  const sessions = useSessionStore((s) => s.sessions);
  const activeId = useSessionStore((s) => s.activeSessionId);
  const setSessions = useSessionStore((s) => s.setSessions);
  const setActive = useSessionStore((s) => s.setActive);
  const setMessages = useMessageStore((s) => s.setMessages);
  const activeMessages = useMessageStore((s) =>
    activeId ? s.messages[activeId] : undefined,
  );
  const setProfile = useAuthStore((s) => s.setProfile);
  const profileUserId = useAuthStore((s) => s.profile?.userId);
  const loadReadCursors = useSessionStore((s) => s.loadReadCursors);
  const users = useUserStore((s) => s.users);
  const upsertUsers = useUserStore((s) => s.upsertUsers);
  const listWidth = useUiStore((s) => s.listWidth);

  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [reloadKey, setReloadKey] = useState(0);
  // 记录已请求过的 userId，避免后端未返回某 id 时重复请求
  const requestedUserIds = useRef<Set<string>>(new Set());

  // 当前用户资料（用于气泡左右判定 / 自己的昵称头像）
  useEffect(() => {
    getProfile()
      .then((rsp) => {
        if (rsp.success === true && rsp.userInfo) setProfile(rsp.userInfo);
      })
      .catch(() => {
        // 资料失败不影响主流程，气泡退化为对方样式
      });
  }, [setProfile]);

  // 资料就绪后加载该账号的已读记录，用于登录后推导未读红点
  useEffect(() => {
    if (profileUserId) loadReadCursors(profileUserId);
  }, [profileUserId, loadReadCursors]);

  // 加载会话列表，默认选中第一个
  useEffect(() => {
    let cancelled = false;
    getChatSessionList()
      .then((rsp) => {
        if (cancelled) return;
        if (rsp.success !== true) {
          setError(rsp.errmsg || "加载会话列表失败");
          setLoading(false);
          return;
        }
        const list = rsp.chatSessionInfoList ?? [];
        setSessions(list);
        const cur = useSessionStore.getState().activeSessionId;
        if (list.length > 0) {
          const stillExists = list.some((s) => s.chatSessionId === cur);
          setActive(stillExists && cur ? cur : list[0].chatSessionId);
        }
        // 列表已渲染：允许 App 建立 WS 连接
        useSessionStore.getState().setSessionsLoaded(true);
        setLoading(false);
      })
      .catch((err: unknown) => {
        if (cancelled) return;
        setError(err instanceof Error ? err.message : "加载会话列表失败");
        setLoading(false);
      });
    return () => {
      cancelled = true;
    };
  }, [reloadKey, setSessions, setActive]);

  // 拉取某会话最近消息（切会话与延迟补齐共用）
  const loadRecent = useCallback(
    async (sessionId: string): Promise<string | null> => {
      try {
        const rsp = await getRecent({ chatSessionId: sessionId, msgCount: 50 });
        if (rsp.success !== true) return rsp.errmsg || "加载消息失败";
        setMessages(sessionId, sortAsc(rsp.msgList ?? []));
        return null;
      } catch (err: unknown) {
        return err instanceof Error ? err.message : "加载消息失败";
      }
    },
    [setMessages],
  );

  // 切换会话时拉取最近消息（切换不清空其他会话已有数据，AGENTS.md §7）
  // 登录/切换只拉一次，后续以 WS 推送为准；不做轮询补拉
  useEffect(() => {
    if (!activeId) return;
    let cancelled = false;
    void loadRecent(activeId).then((err) => {
      if (!cancelled && err) setError(err);
    });
    return () => {
      cancelled = true;
    };
  }, [activeId, loadRecent]);

  // 消息 sender 只有 userId，批量补全缺失用户的昵称 / 头像
  useEffect(() => {
    if (!activeMessages || activeMessages.length === 0) return;
    const ids: string[] = [];
    const seen = new Set<string>();
    for (const msg of activeMessages) {
      const id = msg.sender?.userId;
      if (!id || seen.has(id)) continue;
      seen.add(id);
      if (users[id] || requestedUserIds.current.has(id)) continue;
      ids.push(id);
    }
    if (ids.length === 0) return;

    ids.forEach((id) => requestedUserIds.current.add(id));
    let cancelled = false;
    getMultiUserInfo(ids)
      .then((rsp) => {
        if (cancelled || rsp.success !== true) return;
        upsertUsers(Object.values(rsp.usersInfo ?? {}));
      })
      .catch(() => {
        // 昵称补全失败时组件回退「未知用户」
      });
    return () => {
      cancelled = true;
    };
  }, [activeMessages, users, upsertUsers]);

  const activeSession = sessions.find((s) => s.chatSessionId === activeId);

  // 侧边栏预览已前进但详情里没有这条消息时（离线漏推、补齐请求早于落库等），
  // 以预览消息为准补拉一次，保证侧边栏与详情区一致
  const syncedRef = useRef<string | null>(null);
  const syncRetryRef = useRef<number | undefined>(undefined);
  useEffect(() => {
    if (!activeId) return;
    const prevMessage = activeSession?.prevMessage;
    if (!prevMessage) return;
    const loaded = activeMessages?.some(
      (m) => m.messageId === prevMessage.messageId,
    );
    const key = `${activeId}:${prevMessage.messageId}`;
    if (loaded) {
      syncedRef.current = key;
      return;
    }
    if (syncedRef.current === key) return;
    syncedRef.current = key;
    void loadRecent(activeId);
    // 预览与消息落库存在时序差：稍后仍缺失时再补一次
    window.clearTimeout(syncRetryRef.current);
    syncRetryRef.current = window.setTimeout(() => {
      const msgs = useMessageStore.getState().messages[activeId];
      if (msgs?.some((m) => m.messageId === prevMessage.messageId)) return;
      void loadRecent(activeId);
    }, 1200);
  }, [activeId, activeSession?.prevMessage, activeMessages, loadRecent]);
  useEffect(() => () => window.clearTimeout(syncRetryRef.current), []);

  // 重试：先回到 loading 态再触发重新拉取（避免在 effect 内同步 setState）
  const retry = () => {
    setError(null);
    setLoading(true);
    setReloadKey((k) => k + 1);
  };

  return (
    <div className="flex h-full w-full">
      <Rail />
      <div
        style={{ width: listWidth }}
        className="flex shrink-0 flex-col border-r border-line bg-list"
      >
        <SessionList />
      </div>
      <div className="flex min-w-0 flex-1 flex-col">
        {error ? (
          <ErrorState message={error} onRetry={retry} />
        ) : activeSession ? (
          <>
            <MessageHeader session={activeSession} />
            <MessageList session={activeSession} />
            <InputArea sessionId={activeSession.chatSessionId} />
          </>
        ) : loading ? (
          <div className="flex h-full w-full items-center justify-center bg-app">
            <span className="text-sm text-secondary">加载中…</span>
          </div>
        ) : (
          <EmptyState />
        )}
      </div>
    </div>
  );
}
