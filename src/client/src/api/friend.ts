import { request, USE_MOCK } from "./client";
import { mockGetChatSessionList, mockGetFriendList } from "./mock";
import type { ApiRsp, ChatSessionInfo, UserInfo } from "@/types";

// 会话列表（GET /friend/chat_session/list，无参数，含 chatSessionId 与 prevMessage）
export function getChatSessionList(): Promise<
  ApiRsp & { chatSessionInfoList?: ChatSessionInfo[] }
> {
  if (USE_MOCK) return mockGetChatSessionList();
  return request("/friend/chat_session/list");
}

// 好友列表（GET /friend/list，无参数）——本轮不用于会话列表，保留接口
export function getFriendList(): Promise<ApiRsp & { friendList?: UserInfo[] }> {
  if (USE_MOCK) return mockGetFriendList();
  return request("/friend/list");
}
