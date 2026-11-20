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

  const publish = async (
    messageId: string,
    message: LocalMessage["message"],
  ) => {
    try {
      const rsp = await publishMessage({
        chatSessionId: sessionId,
        message: {
          ...message,
          messageType:
            message.messageType ?? (message.fileMessage ? "FILE" : "STRING"),
        },
      });
      if (rsp.success === true) {
        markSent(sessionId, messageId);
        // 自己发的消息不会收到 NEW_MESSAGE 推送，需在此同步会话预览
        const sent = useMessageStore
          .getState()
          .messages[sessionId]?.find((m) => m.messageId === messageId);
        if (sent) {
          useSessionStore.getState().updatePrevMessage(sessionId, sent);
          // 仍停留在该会话时，自己发的消息不点亮未读红点
          if (useSessionStore.getState().activeSessionId === sessionId) {
            useSessionStore.getState().markSessionRead(sessionId, sent);
          }
        }
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
    await publish(tempId, msg.message);
    return true;
  };

  // 发送文件（图片也走这里）：fileContent 为 base64，由消息存储服务转存对象存储
  const sendFile = async (
    fileName: string,
    fileSize: number,
    fileContent: string,
  ): Promise<boolean> => {
    if (!myId || !fileContent) return false;

    const tempId = `temp-${crypto.randomUUID()}`;
    const msg: LocalMessage = {
      messageId: tempId,
      chatSessionId: sessionId,
      sender: { userId: myId },
      message: {
        messageType: "FILE",
        fileMessage: {
          fileName,
          fileSize: String(fileSize),
          fileContent,
        },
      },
      createTime: dayjs().unix().toString(),
      status: "sending",
    };
    appendOptimistic(sessionId, msg);
    await publish(tempId, msg.message);
    return true;
  };

  // 发送多张图片：逐张发送，保证消息顺序与选择顺序一致
  const sendImages = async (
    images: { fileName: string; fileSize: number; content: string }[],
  ) => {
    for (const img of images) {
      await sendFile(img.fileName, img.fileSize, img.content);
    }
  };

  // 重新发送失败消息：先复位为 sending，避免重复点击重发
  const retry = async (msg: LocalMessage) => {
    if (msg.status !== "failed") return;
    markSending(sessionId, msg.messageId);
    await publish(msg.messageId, msg.message);
  };

  return { send, sendFile, sendImages, retry };
}
