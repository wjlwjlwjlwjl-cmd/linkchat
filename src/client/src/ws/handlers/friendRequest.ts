import { getMultiUserInfo } from "@/api/user";
import { useFriendStore } from "@/stores/friend";

// FRIEND_REQUEST：推送只带 applyUserId，补全资料后记录待处理申请（eventId 去重）
export function handleFriendRequest(data: unknown): void {
  if (typeof data !== "object" || data === null) return;
  const o = data as { notifyEventId?: unknown; applyUserId?: unknown };
  if (typeof o.applyUserId !== "string" || !o.applyUserId) return;
  const applyUserId = o.applyUserId;
  const eventId = typeof o.notifyEventId === "string" ? o.notifyEventId : "";
  // 没有事件 id 无法同意/拒绝，直接忽略
  if (!eventId) return;

  const store = useFriendStore.getState();
  if (store.pendingApplies.some((e) => e.eventId === eventId)) return;

  void getMultiUserInfo([applyUserId])
    .then((rsp) => {
      const sender = rsp.usersInfo?.[applyUserId] ?? { userId: applyUserId };
      useFriendStore.getState().addPendingApply({ eventId, sender });
    })
    .catch(() => {
      // 资料拉取失败时先用纯 userId 占位，资料行会显示首字头像
      useFriendStore.getState().addPendingApply({
        eventId,
        sender: { userId: applyUserId },
      });
    });
}
