import { request, USE_MOCK } from "./client";
import {
  mockAddFriend,
  mockGetChatSessionList,
  mockGetChatSessionMembers,
  mockGetFriendList,
  mockGetPendingFriendEvents,
  mockGetSingleChatSession,
  mockProcessFriendEvent,
  mockSearchFriends,
} from "./mock";
import type { ApiRsp, ChatSessionInfo, FriendEvent, UserInfo } from "@/types";

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

// 用户搜索（GET /friend/search?search_key=&count=，昵称模糊匹配）
export function searchFriends(
  searchKey: string,
  count = 20,
): Promise<ApiRsp & { userInfo?: UserInfo[] }> {
  if (USE_MOCK) return mockSearchFriends(searchKey, count);
  const query = new URLSearchParams({
    search_key: searchKey,
    count: String(count),
  });
  return request(`/friend/search?${query.toString()}`);
}

// 会话成员（GET /friend/chat_session/member?chat_session_id=）
export function getChatSessionMembers(
  chatSessionId: string,
): Promise<ApiRsp & { memberInfoList?: UserInfo[] }> {
  if (USE_MOCK) return mockGetChatSessionMembers(chatSessionId);
  const query = new URLSearchParams({ chat_session_id: chatSessionId });
  return request(`/friend/chat_session/member?${query.toString()}`);
}

// 获取或创建与好友的单聊会话（GET /friend/chat_session/single?friend_id=）
export function getSingleChatSession(
  friendId: string,
): Promise<ApiRsp & { chatSessionId?: string }> {
  if (USE_MOCK) return mockGetSingleChatSession(friendId);
  const query = new URLSearchParams({ friend_id: friendId });
  return request(`/friend/chat_session/single?${query.toString()}`);
}

// 发起好友申请（POST /friend/add，body: respondent_id）
export function addFriend(
  respondentId: string,
): Promise<ApiRsp & { notifyEventId?: string }> {
  if (USE_MOCK) return mockAddFriend(respondentId);
  return request("/friend/add", {
    method: "POST",
    body: JSON.stringify({ respondent_id: respondentId }),
  });
}

// 待处理好友申请（GET /friend/pending_event）
export function getPendingFriendEvents(): Promise<
  ApiRsp & { event?: FriendEvent[] }
> {
  if (USE_MOCK) return mockGetPendingFriendEvents();
  return request("/friend/pending_event");
}

// 处理好友申请（POST /friend/add_process，同意后后端会创建单聊会话）
export function processFriendEvent(
  notifyEventId: string,
  agree: boolean,
  applyUserId: string,
): Promise<ApiRsp & { newSessionId?: string }> {
  if (USE_MOCK)
    return mockProcessFriendEvent(notifyEventId, agree, applyUserId);
  return request("/friend/add_process", {
    method: "POST",
    body: JSON.stringify({
      notify_event_id: notifyEventId,
      agree,
      apply_user_id: applyUserId,
    }),
  });
}
