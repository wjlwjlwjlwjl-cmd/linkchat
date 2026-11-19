import dayjs from "dayjs";
import { publishMessage } from "@/api/message";
import { useAuthStore } from "@/stores/auth";
import { useMessageStore } from "@/stores/message";
import { useSessionStore } from "@/stores/session";
import type { LocalMessage } from "@/types";

// 消息发送流程（AGENTS.md §6.6，写死）：
// temp id → 乐观插入 sending → publishMessage → 成功 sent / 失败 failed
export function useSendMessage(sessionId: string) {
  const appendOptimistic = useMessageStore((s) => s.appendOptimistic);
  const markSent = useMessageStore((s) => s.markSent);
  const markSending = useMessageStore((s) => s.markSending);
  const markFailed = useMessageStore((s) => s.markFailed);
  const myId = useAuthStore((s) => s.profile?.userId);

  const publish = async (messageId: string, content: string) => {
    try {
      const rsp = await publishMessage({
        chatSessionId: sessionId,
        message: {
          messageType: "STRING",
          stringMessage: { content },
        },
      });
      if (rsp.success === true) {
        markSent(sessionId, messageId);
        // 自己发的消息不会收到 NEW_MESSAGE 推送，需在此同步会话预览
        const sent = useMessageStore
          .getState()
          .messages[sessionId]?.find((m) => m.messageId === messageId);
        if (sent) useSessionStore.getState().updatePrevMessage(sessionId, sent);
      } else {
        markFailed(sessionId, messageId);
      }
    } catch {
      markFailed(sessionId, messageId);
    }
  };

  // 返回是否已发起发送（空内容 / 未登录返回 false）
  const send = async (raw: string): Promise<boolean> => {
    const content = raw.trim();
    if (!content || !myId) return false;

    const tempId = `temp-${crypto.randomUUID()}`;
    const msg: LocalMessage = {
      messageId: tempId,
      chatSessionId: sessionId,
      sender: { userId: myId },
      message: { messageType: "STRING", stringMessage: { content } },
      createTime: dayjs().unix().toString(),
      status: "sending",
    };
    appendOptimistic(sessionId, msg);
    await publish(tempId, content);
    return true;
  };

  // 重新发送失败消息：先复位为 sending，避免重复点击重发
  const retry = async (msg: LocalMessage) => {
    if (msg.status !== "failed") return;
    const content = msg.message.stringMessage?.content ?? "";
    markSending(sessionId, msg.messageId);
    await publish(msg.messageId, content);
  };

  return { send, retry };
}
