import { create } from "zustand";
import type { UserInfo } from "@/types";

const TOKEN_KEY = "linkchat_token";

// 登录态：token + 当前用户资料（AGENTS.md §7，401 时 clear 触发回登录页）
interface AuthState {
  token: string;
  profile: UserInfo | null;
  setToken: (token: string) => void;
  setProfile: (profile: UserInfo) => void;
  clear: () => void;
}

export const useAuthStore = create<AuthState>((set) => ({
  token: localStorage.getItem(TOKEN_KEY) ?? "",
  profile: null,
  setToken: (token) => {
    localStorage.setItem(TOKEN_KEY, token);
    set({ token });
  },
  setProfile: (profile) => set({ profile }),
  clear: () => {
    localStorage.removeItem(TOKEN_KEY);
    set({ token: "", profile: null });
  },
}));
