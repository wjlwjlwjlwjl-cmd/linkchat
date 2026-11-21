import { create } from "zustand";

// 预签名 URL 有效期 15 分钟，缓存 10 分钟后视为过期
export const FILE_URL_TTL = 10 * 60 * 1000;

interface FileUrlEntry {
  url: string;
  expiresAt: number;
}

interface FileUrlState {
  urls: Record<string, FileUrlEntry>;
  setUrls: (entries: Record<string, FileUrlEntry>) => void;
}

// 历史消息的文件地址：由消息列表通过批量接口统一填充，避免逐条调用单个文件接口
export const useFileUrlStore = create<FileUrlState>((set) => ({
  urls: {},
  setUrls: (entries) =>
    set((state) => ({ urls: { ...state.urls, ...entries } })),
}));
