import { useSessionStore } from "@/stores/session";

// FRIEND_APPLY_RESULT：对方同意后刷新会话列表（推送只带 newSessionId，需拉全量）
export function handleFriendApplyResult(data: unknown): void {
  if (typeof data !== "object" || data === null) return;
  const o = data as { agree?: unknown };
  if (o.agree !== true) return;
  void useSessionStore.getState().refresh();
}
