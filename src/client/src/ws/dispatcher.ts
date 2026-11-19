import type { MsgType } from "@/types";
import { handleNewMessage } from "./handlers/newMessage";
import { handleFriendRequest } from "./handlers/friendRequest";
import { handleFriendApplyResult } from "./handlers/friendApplyResult";
import { handleChatSessionCreated } from "./handlers/chatSessionCreated";

export type Handler = (data: unknown) => void;

// 推送分发表（AGENTS.md §6.8）：所有推送统一在此登记
export const handlers: Record<MsgType, Handler> = {
  NEW_MESSAGE: handleNewMessage,
  FRIEND_REQUEST: handleFriendRequest,
  FRIEND_APPLY_RESULT: handleFriendApplyResult,
  CHAT_SESSION_CREATED: handleChatSessionCreated,
};

function isMsgType(v: string): v is MsgType {
  return Object.prototype.hasOwnProperty.call(handlers, v);
}

// 解析 text 帧并分发；非 JSON / 缺 type / 未知 type 一律静默丢弃
export function dispatch(raw: string): void {
  let parsed: unknown;
  try {
    parsed = JSON.parse(raw);
  } catch {
    return;
  }
  if (typeof parsed !== "object" || parsed === null) return;
  const env = parsed as { type?: unknown; data?: unknown };
  if (typeof env.type !== "string" || !isMsgType(env.type)) return;
  handlers[env.type](env.data);
}
