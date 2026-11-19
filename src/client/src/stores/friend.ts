import { create } from "zustand";

// 好友申请推送状态（FRIEND_REQUEST）；通知事件 id 用于去重
interface FriendState {
  pendingApplyUserIds: string[];
  notifyEventIds: string[];
  addPendingApply: (userId: string, eventId: string) => void;
}

export const useFriendStore = create<FriendState>((set) => ({
  pendingApplyUserIds: [],
  notifyEventIds: [],
  addPendingApply: (userId, eventId) =>
    set((state) => {
      if (!userId) return state;
      if (eventId && state.notifyEventIds.includes(eventId)) return state;
      return {
        pendingApplyUserIds: state.pendingApplyUserIds.includes(userId)
          ? state.pendingApplyUserIds
          : [...state.pendingApplyUserIds, userId],
        notifyEventIds: eventId
          ? [...state.notifyEventIds, eventId]
          : state.notifyEventIds,
      };
    }),
}));
