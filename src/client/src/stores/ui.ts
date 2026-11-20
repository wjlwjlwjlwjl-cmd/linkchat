import { create } from "zustand";
import type { UserInfo } from "@/types";

// 主题模式：浅色 / 深色 / 跟随系统（style.md §5）
export type ThemeMode = "light" | "dark" | "system";

const THEME_KEY = "linkchat_theme";

// 左侧栏视图：会话列表 / 通讯录（由功能栏切换）
export type SideView = "chats" | "contacts";

function readThemeMode(): ThemeMode {
  try {
    const value = localStorage.getItem(THEME_KEY);
    return value === "light" || value === "dark" || value === "system"
      ? value
      : "system";
  } catch {
    return "system";
  }
}

function systemPrefersDark(): boolean {
  return window.matchMedia("(prefers-color-scheme: dark)").matches;
}

// 主题只通过 <html data-theme> 生效，组件不感知主题
function applyTheme(mode: ThemeMode, systemDark: boolean): void {
  const dark = mode === "dark" || (mode === "system" && systemDark);
  const root = document.documentElement;
  root.dataset.theme = dark ? "dark" : "light";
  root.style.colorScheme = dark ? "dark" : "light";
}

// UI 状态：会话列表拖拽宽度（AGENTS.md §2.4，M0 静态默认 260）
interface UiState {
  listWidth: number;
  setListWidth: (width: number) => void;
  // WS 连接异常（连接失败/断线），用于侧栏顶部网络警告
  wsError: boolean;
  setWsError: (wsError: boolean) => void;
  // 侧边栏是否展示「聊天信息」面板（点击标题栏更多按钮切换）
  infoOpen: boolean;
  setInfoOpen: (infoOpen: boolean) => void;
  // 左侧栏展示会话列表还是通讯录
  sideView: SideView;
  setSideView: (sideView: SideView) => void;
  // 通讯录中选中的好友（在详情区展示资料页）
  selectedContact: UserInfo | null;
  setSelectedContact: (contact: UserInfo | null) => void;
  themeMode: ThemeMode;
  systemDark: boolean;
  setThemeMode: (themeMode: ThemeMode) => void;
  // 系统深浅色变化时同步（仅在「跟随系统」下影响外观）
  syncSystemTheme: () => void;
}

const MIN = 220;
const MAX = 360;

export const useUiStore = create<UiState>((set, get) => ({
  listWidth: 260,
  setListWidth: (width) =>
    set({ listWidth: Math.min(MAX, Math.max(MIN, width)) }),
  wsError: false,
  setWsError: (wsError) => set({ wsError }),
  infoOpen: false,
  setInfoOpen: (infoOpen) => set({ infoOpen }),
  sideView: "chats",
  setSideView: (sideView) => set({ sideView }),
  selectedContact: null,
  setSelectedContact: (selectedContact) => set({ selectedContact }),
  themeMode: readThemeMode(),
  systemDark: systemPrefersDark(),
  setThemeMode: (themeMode) => {
    applyTheme(themeMode, get().systemDark);
    try {
      localStorage.setItem(THEME_KEY, themeMode);
    } catch {
      // 存储失败时仅本次会话生效
    }
    set({ themeMode });
  },
  syncSystemTheme: () => {
    const systemDark = systemPrefersDark();
    if (systemDark === get().systemDark) return;
    applyTheme(get().themeMode, systemDark);
    set({ systemDark });
  },
}));
