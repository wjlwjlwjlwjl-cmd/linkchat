import { useFriendStore } from "@/stores/friend";

// FRIEND_REQUEST：记录待处理申请（notifyEventId 去重）
export function handleFriendRequest(data: unknown): void {
  if (typeof data !== "object" || data === null) return;
  const o = data as { notifyEventId?: unknown; applyUserId?: unknown };
  if (typeof o.applyUserId !== "string" || !o.applyUserId) return;
  const eventId = typeof o.notifyEventId === "string" ? o.notifyEventId : "";
  useFriendStore.getState().addPendingApply(o.applyUserId, eventId);
}
