import { create } from "zustand";
import type { LocalMessage, MessageInfo } from "@/types";

// 消息按会话存 Record<sessionId, LocalMessage[]>，切换会话不清空（AGENTS.md §7）
// store action 契约见 §6.5：appendOptimistic / markFailed / dedupeById
// 补充：markSent（§6.6 成功态）、markSending（重发前复位）
interface MessageState {
  messages: Record<string, LocalMessage[]>;
  setMessages: (sessionId: string, list: MessageInfo[]) => void;
  appendMessage: (sessionId: string, msg: MessageInfo) => void;
  appendOptimistic: (sessionId: string, msg: LocalMessage) => void;
  markSent: (sessionId: string, messageId: string) => void;
  // 发送成功后用服务端 messageId 替换本地临时 id，保证后续拉取能按 id 去重
  confirmSent: (sessionId: string, tempId: string, serverId: string) => void;
  markSending: (sessionId: string, messageId: string) => void;
  markFailed: (sessionId: string, tempId: string, failReason?: string) => void;
  dedupeById: (sessionId: string, incoming: MessageInfo[]) => void;
}

// 按 createTime 升序，同时间按 messageId（临时 id 与服务端 id 不混用）
function compare(a: LocalMessage, b: LocalMessage): number {
  const ta = Number(a.createTime);
  const tb = Number(b.createTime);
  if (ta !== tb) return ta - tb;
  return a.messageId.localeCompare(b.messageId);
}

function toLocal(msg: MessageInfo): LocalMessage {
  return { ...msg, status: "sent" };
}

function updateStatus(
  messages: Record<string, LocalMessage[]>,
  sessionId: string,
  messageId: string,
  status: LocalMessage["status"],
): Record<string, LocalMessage[]> {
  const cur = messages[sessionId];
  if (!cur) return messages;
  return {
    ...messages,
    [sessionId]: cur.map((m) =>
      m.messageId === messageId ? { ...m, status } : m,
    ),
  };
}

export const useMessageStore = create<MessageState>((set) => ({
  messages: {},
  // 服务端列表按 messageId 合并：保留已推送消息与本地 temp- 消息，
  // 避免「推送先到、点开会话时后端尚未落库」导致消息被覆盖丢失
  setMessages: (sessionId, list) =>
    set((state) => {
      const byId = new Map(
        (state.messages[sessionId] ?? []).map((m) => [m.messageId, m]),
      );
      for (const msg of list) byId.set(msg.messageId, toLocal(msg));
      const merged = Array.from(byId.values()).sort(compare);
      return { messages: { ...state.messages, [sessionId]: merged } };
    }),
  appendMessage: (sessionId, msg) =>
    set((state) => {
      const cur = state.messages[sessionId] ?? [];
      return {
        messages: {
          ...state.messages,
          [sessionId]: [...cur, toLocal(msg)].sort(compare),
        },
      };
    }),
  appendOptimistic: (sessionId, msg) =>
    set((state) => {
      const cur = state.messages[sessionId] ?? [];
      return {
        messages: { ...state.messages, [sessionId]: [...cur, msg] },
      };
    }),
  markSent: (sessionId, messageId) =>
    set((state) => ({
      messages: updateStatus(state.messages, sessionId, messageId, "sent"),
    })),
  confirmSent: (sessionId, tempId, serverId) =>
    set((state) => {
      const cur = state.messages[sessionId];
      if (!cur) return state;
      // 服务端副本可能已通过重连补齐先到：此时直接丢弃本地临时消息
      const hasServer = cur.some((m) => m.messageId === serverId);
      let replaced = false;
      const next: LocalMessage[] = [];
      for (const m of cur) {
        if (m.messageId !== tempId) {
          next.push(m);
          continue;
        }
        replaced = true;
        if (!hasServer) {
          next.push({ ...m, messageId: serverId, status: "sent" });
        }
      }
      if (!replaced) return state;
      return {
        messages: { ...state.messages, [sessionId]: next.sort(compare) },
      };
    }),
  markSending: (sessionId, messageId) =>
    set((state) => {
      const cur = state.messages[sessionId];
      if (!cur) return state;
      return {
        messages: {
          ...state.messages,
          [sessionId]: cur.map((m) =>
            m.messageId === messageId
              ? { ...m, status: "sending", failReason: undefined }
              : m,
          ),
        },
      };
    }),
  markFailed: (sessionId, tempId, failReason) =>
    set((state) => {
      const cur = state.messages[sessionId];
      if (!cur) return state;
      return {
        messages: {
          ...state.messages,
          [sessionId]: cur.map((m) =>
            m.messageId === tempId ? { ...m, status: "failed", failReason } : m,
          ),
        },
      };
    }),
  // 只对服务端 messageId 去重，临时 id 不参与
  dedupeById: (sessionId, incoming) =>
    set((state) => {
      const cur = state.messages[sessionId] ?? [];
      const existing = new Set(cur.map((m) => m.messageId));
      const merged = [
        ...cur,
        ...incoming.filter((m) => !existing.has(m.messageId)).map(toLocal),
      ].sort(compare);
      return { messages: { ...state.messages, [sessionId]: merged } };
    }),
}));
