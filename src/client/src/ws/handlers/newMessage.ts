import { useMessageStore } from "@/stores/message";
import { useSessionStore } from "@/stores/session";
import type { MessageInfo } from "@/types";

function isMessageInfo(v: unknown): v is MessageInfo {
  if (typeof v !== "object" || v === null) return false;
  const o = v as Record<string, unknown>;
  return (
    typeof o.messageId === "string" &&
    typeof o.chatSessionId === "string" &&
    typeof o.sender === "object" &&
    o.sender !== null &&
    typeof o.message === "object" &&
    o.message !== null
  );
}

// NEW_MESSAGE：去重入库并更新该会话的预览消息
export function handleNewMessage(data: unknown): void {
  if (!isMessageInfo(data)) return;
  const sessionId = data.chatSessionId;
  useMessageStore.getState().dedupeById(sessionId, [data]);

  const session = useSessionStore
    .getState()
    .sessions.find((s) => s.chatSessionId === sessionId);
  if (session) {
    useSessionStore.getState().updatePrevMessage(sessionId, data);
  } else {
    // 未知会话（新会话）：刷新列表
    void useSessionStore.getState().refresh();
  }
}
