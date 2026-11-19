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

  const sessionState = useSessionStore.getState();
  const session = sessionState.sessions.find(
    (s) => s.chatSessionId === sessionId,
  );
  if (session) {
    sessionState.updatePrevMessage(sessionId, data);
    // 正在查看该会话：消息已展示，直接视为已读，不显示红点
    if (sessionState.activeSessionId === sessionId) {
      sessionState.markSessionRead(sessionId, data);
    }
    return;
  }
  // 未知会话（新会话）：刷新列表，并让该会话保持未读
  void sessionState.refresh().then(() => {
    useSessionStore.getState().markSessionUnread(sessionId);
  });
}
