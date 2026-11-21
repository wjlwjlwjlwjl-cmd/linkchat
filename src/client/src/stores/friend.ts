import { create } from "zustand";
import type { FriendEvent } from "@/types";

// 待处理好友申请：WS 推送 + GET /friend/pending_event 全量获取，共用同一份列表
interface FriendState {
  pendingApplies: FriendEvent[];
  // 同意申请后自增，通讯录据此重新拉取好友列表
  friendListVersion: number;
  // 全量覆盖（保留尚未出现在服务端结果里的推送，按 eventId 去重）
  setPendingApplies: (events: FriendEvent[]) => void;
  addPendingApply: (event: FriendEvent) => void;
  removePendingApply: (eventId: string) => void;
  bumpFriendListVersion: () => void;
}

export const useFriendStore = create<FriendState>((set) => ({
  pendingApplies: [],
  friendListVersion: 0,
  setPendingApplies: (events) =>
    set((state) => {
      const seen = new Set(events.map((e) => e.eventId));
      const extra = state.pendingApplies.filter((e) => !seen.has(e.eventId));
      return { pendingApplies: [...events, ...extra] };
    }),
  addPendingApply: (event) =>
    set((state) => {
      if (
        !event.eventId ||
        state.pendingApplies.some((e) => e.eventId === event.eventId)
      ) {
        return state;
      }
      return { pendingApplies: [...state.pendingApplies, event] };
    }),
  removePendingApply: (eventId) =>
    set((state) => ({
      pendingApplies: state.pendingApplies.filter((e) => e.eventId !== eventId),
    })),
  bumpFriendListVersion: () =>
    set((state) => ({ friendListVersion: state.friendListVersion + 1 })),
}));
