import { useSessionStore } from "@/stores/session";

// CHAT_SESSION_CREATED：新会话创建，刷新会话列表（§10：推送只带 id，需拉全量）
export function handleChatSessionCreated(data: unknown): void {
  if (typeof data !== "object" || data === null) return;
  void useSessionStore.getState().refresh();
}
