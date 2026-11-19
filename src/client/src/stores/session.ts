import { create } from "zustand";
import { getChatSessionList } from "@/api/friend";
import type { ChatSessionInfo, MessageInfo } from "@/types";

// 会话列表 + 当前选中会话（AGENTS.md §7）
interface SessionState {
  sessions: ChatSessionInfo[];
  activeSessionId: string | null;
  setSessions: (sessions: ChatSessionInfo[]) => void;
  setActive: (id: string) => void;
  // 推送只带 id 时主动刷新列表（§10）
  refresh: () => Promise<void>;
  updatePrevMessage: (sessionId: string, msg: MessageInfo) => void;
}

export const useSessionStore = create<SessionState>((set) => ({
  sessions: [],
  activeSessionId: null,
  setSessions: (sessions) => set({ sessions }),
  setActive: (activeSessionId) => set({ activeSessionId }),
  refresh: async () => {
    try {
      const rsp = await getChatSessionList();
      if (rsp.success !== true) return;
      const list = rsp.chatSessionInfoList ?? [];
      set((state) => {
        const active = state.activeSessionId;
        const stillExists =
          active !== null && list.some((s) => s.chatSessionId === active);
        return {
          sessions: list,
          activeSessionId: stillExists
            ? active
            : (list[0]?.chatSessionId ?? null),
        };
      });
    } catch {
      // 刷新失败保留旧列表，下次推送/重连再试
    }
  },
  updatePrevMessage: (sessionId, msg) =>
    set((state) => ({
      sessions: state.sessions.map((s) =>
        s.chatSessionId === sessionId ? { ...s, prevMessage: msg } : s,
      ),
    })),
}));
