import { create } from "zustand";
import { getChatSessionList } from "@/api/friend";
import { useAuthStore } from "@/stores/auth";
import type { ChatSessionInfo, MessageInfo } from "@/types";

// 已读位置：记录读到该会话的哪条消息，与消息列表使用同一排序规则
export interface ReadCursor {
  createTime: string;
  messageId: string;
}

const READ_KEY_PREFIX = "linkchat_read_";
const PIN_KEY_PREFIX = "linkchat_pin_";

// 与消息列表一致：先比较秒级时间，同秒再比较 messageId
function compareMessage(
  a: { createTime: string; messageId: string },
  b: { createTime: string; messageId: string },
): number {
  const timeDiff = Number(a.createTime) - Number(b.createTime);
  return timeDiff !== 0 ? timeDiff : a.messageId.localeCompare(b.messageId);
}

function cursorOf(msg: MessageInfo): ReadCursor {
  return { createTime: msg.createTime, messageId: msg.messageId };
}

// 推进已读位置；没有变化时返回原对象，避免无意义重渲染
function advanceCursor(
  cursors: Record<string, ReadCursor>,
  sessionId: string,
  msg: MessageInfo,
): Record<string, ReadCursor> {
  const prev = cursors[sessionId];
  if (prev && compareMessage(msg, prev) <= 0) return cursors;
  return { ...cursors, [sessionId]: cursorOf(msg) };
}

// 首次见到某会话（无已读记录）时，把当前预览视为已读基线
function seedCursors(
  cursors: Record<string, ReadCursor>,
  sessions: ChatSessionInfo[],
): Record<string, ReadCursor> {
  let next = cursors;
  for (const session of sessions) {
    if (!session.prevMessage || next[session.chatSessionId]) continue;
    if (next === cursors) next = { ...cursors };
    next[session.chatSessionId] = cursorOf(session.prevMessage);
  }
  return next;
}

// 是否显示未读红点：预览消息比已读位置更新（无记录视为未读）
export function hasUnread(
  session: ChatSessionInfo,
  cursor?: ReadCursor,
): boolean {
  if (!session.prevMessage) return false;
  if (!cursor) return true;
  return compareMessage(session.prevMessage, cursor) > 0;
}

function loadStored(userId: string): Record<string, ReadCursor> {
  try {
    const raw = localStorage.getItem(READ_KEY_PREFIX + userId);
    if (!raw) return {};
    const parsed: unknown = JSON.parse(raw);
    if (typeof parsed !== "object" || parsed === null) return {};
    const result: Record<string, ReadCursor> = {};
    for (const [sessionId, value] of Object.entries(parsed)) {
      if (typeof value !== "object" || value === null) continue;
      const item = value as { createTime?: unknown; messageId?: unknown };
      if (
        typeof item.createTime === "string" &&
        typeof item.messageId === "string"
      ) {
        result[sessionId] = {
          createTime: item.createTime,
          messageId: item.messageId,
        };
      }
    }
    return result;
  } catch {
    return {};
  }
}

// 按当前登录账号持久化，切换账号不会串数据
function saveStored(cursors: Record<string, ReadCursor>): void {
  const userId = useAuthStore.getState().profile?.userId;
  if (!userId) return;
  try {
    localStorage.setItem(READ_KEY_PREFIX + userId, JSON.stringify(cursors));
  } catch {
    // 存储失败只影响下次登录的红点推导，不影响本次会话
  }
}

// 置顶会话按账号持久化（后端无置顶字段）
function loadPinnedStored(userId: string): string[] {
  try {
    const raw = localStorage.getItem(PIN_KEY_PREFIX + userId);
    if (!raw) return [];
    const parsed: unknown = JSON.parse(raw);
    if (!Array.isArray(parsed)) return [];
    return parsed.filter((item): item is string => typeof item === "string");
  } catch {
    return [];
  }
}

function savePinnedStored(ids: string[]): void {
  const userId = useAuthStore.getState().profile?.userId;
  if (!userId) return;
  try {
    localStorage.setItem(PIN_KEY_PREFIX + userId, JSON.stringify(ids));
  } catch {
    // 存储失败仅影响下次登录的置顶状态
  }
}

// 会话列表 + 当前选中会话 + 已读位置（AGENTS.md §7）
interface SessionState {
  sessions: ChatSessionInfo[];
  activeSessionId: string | null;
  // 会话列表是否已成功加载（用于串行化：列表渲染后再建 WS）
  sessionsLoaded: boolean;
  readCursors: Record<string, ReadCursor>;
  // 置顶会话 id（本地持久化）
  pinnedIds: string[];
  loadPinned: (userId: string) => void;
  togglePin: (sessionId: string) => void;
  setSessions: (sessions: ChatSessionInfo[]) => void;
  setSessionsLoaded: (loaded: boolean) => void;
  setActive: (id: string) => void;
  // 推送只带 id 时主动刷新列表（§10）
  refresh: () => Promise<void>;
  updatePrevMessage: (sessionId: string, msg: MessageInfo) => void;
  // 登录后按账号加载已读记录
  loadReadCursors: (userId: string) => void;
  markSessionRead: (sessionId: string, msg?: MessageInfo) => void;
  markSessionUnread: (sessionId: string) => void;
}

export const useSessionStore = create<SessionState>((set) => ({
  sessions: [],
  activeSessionId: null,
  sessionsLoaded: false,
  readCursors: {},
  pinnedIds: [],
  loadPinned: (userId) => set({ pinnedIds: loadPinnedStored(userId) }),
  togglePin: (sessionId) =>
    set((state) => {
      const pinnedIds = state.pinnedIds.includes(sessionId)
        ? state.pinnedIds.filter((id) => id !== sessionId)
        : [...state.pinnedIds, sessionId];
      savePinnedStored(pinnedIds);
      return { pinnedIds };
    }),
  setSessions: (sessions) =>
    set((state) => {
      const readCursors = seedCursors(state.readCursors, sessions);
      if (readCursors !== state.readCursors) saveStored(readCursors);
      return { sessions, readCursors };
    }),
  setSessionsLoaded: (sessionsLoaded) => set({ sessionsLoaded }),
  setActive: (activeSessionId) =>
    set((state) => {
      const session = state.sessions.find(
        (s) => s.chatSessionId === activeSessionId,
      );
      if (!session?.prevMessage) return { activeSessionId };
      const readCursors = advanceCursor(
        state.readCursors,
        activeSessionId,
        session.prevMessage,
      );
      if (readCursors !== state.readCursors) saveStored(readCursors);
      return { activeSessionId, readCursors };
    }),
  refresh: async () => {
    try {
      const rsp = await getChatSessionList();
      if (rsp.success !== true) return;
      const list = rsp.chatSessionInfoList ?? [];
      set((state) => {
        const active = state.activeSessionId;
        const stillExists =
          active !== null && list.some((s) => s.chatSessionId === active);
        const activeSessionId = stillExists
          ? active
          : (list[0]?.chatSessionId ?? null);
        let readCursors = seedCursors(state.readCursors, list);
        const activeSession = list.find(
          (s) => s.chatSessionId === activeSessionId,
        );
        if (activeSessionId && activeSession?.prevMessage) {
          readCursors = advanceCursor(
            readCursors,
            activeSessionId,
            activeSession.prevMessage,
          );
        }
        if (readCursors !== state.readCursors) saveStored(readCursors);
        return { sessions: list, activeSessionId, readCursors };
      });
    } catch {
      // 刷新失败保留旧列表，下次推送/重连再试
    }
  },
  updatePrevMessage: (sessionId, msg) =>
    set((state) => ({
      sessions: state.sessions.map((s) => {
        if (s.chatSessionId !== sessionId) return s;
        // 服务端推送可能乱序，旧预览不能覆盖新预览；本地发送始终立即更新
        if (
          !msg.messageId.startsWith("temp-") &&
          s.prevMessage &&
          compareMessage(s.prevMessage, msg) > 0
        ) {
          return s;
        }
        return { ...s, prevMessage: msg };
      }),
    })),
  loadReadCursors: (userId) =>
    set((state) => {
      let readCursors = seedCursors(loadStored(userId), state.sessions);
      const active = state.activeSessionId;
      const activeSession = state.sessions.find(
        (s) => s.chatSessionId === active,
      );
      if (active && activeSession?.prevMessage) {
        readCursors = advanceCursor(
          readCursors,
          active,
          activeSession.prevMessage,
        );
      }
      saveStored(readCursors);
      return { readCursors };
    }),
  markSessionRead: (sessionId, msg) =>
    set((state) => {
      const target =
        msg ??
        state.sessions.find((s) => s.chatSessionId === sessionId)?.prevMessage;
      if (!target) return {};
      const readCursors = advanceCursor(state.readCursors, sessionId, target);
      if (readCursors === state.readCursors) return {};
      saveStored(readCursors);
      return { readCursors };
    }),
  markSessionUnread: (sessionId) =>
    set((state) => {
      if (!(sessionId in state.readCursors)) return {};
      const readCursors = { ...state.readCursors };
      delete readCursors[sessionId];
      saveStored(readCursors);
      return { readCursors };
    }),
}));
