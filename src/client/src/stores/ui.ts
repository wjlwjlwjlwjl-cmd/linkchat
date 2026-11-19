import { create } from "zustand";

// UI 状态：会话列表拖拽宽度（AGENTS.md §2.4，M0 静态默认 260）
interface UiState {
  listWidth: number;
  setListWidth: (width: number) => void;
}

const MIN = 220;
const MAX = 360;

export const useUiStore = create<UiState>((set) => ({
  listWidth: 260,
  setListWidth: (width) =>
    set({ listWidth: Math.min(MAX, Math.max(MIN, width)) }),
}));
