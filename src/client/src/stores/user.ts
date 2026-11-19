import { create } from "zustand";
import type { UserInfo } from "@/types";

// 用户资料缓存：消息里 sender 只有 userId，靠此表补全昵称 / 头像
interface UserState {
  users: Record<string, UserInfo>;
  upsertUsers: (list: UserInfo[]) => void;
}

export const useUserStore = create<UserState>((set) => ({
  users: {},
  upsertUsers: (list) =>
    set((state) => {
      if (list.length === 0) return state;
      const next = { ...state.users };
      for (const u of list) {
        if (u && u.userId) next[u.userId] = { ...next[u.userId], ...u };
      }
      return { users: next };
    }),
}));
