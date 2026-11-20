import { request, USE_MOCK } from "./client";
import {
  mockGetHistory,
  mockGetRecent,
  mockPublishMessage,
  mockSearchMessages,
} from "./mock";
import type { ApiRsp, MessageInfo, PublishReq } from "@/types";

// 某时间段历史消息（GET /message/history，时间参数为 Unix 分钟，闭区间）
export function getHistory(p: {
  chatSessionId: string;
  startTime: number; // Unix 秒，内部 /60 转分钟
  overTime: number; // Unix 秒，内部 /60 转分钟
}): Promise<ApiRsp & { msgList?: MessageInfo[] }> {
  if (USE_MOCK) return mockGetHistory(p);
  const params = new URLSearchParams({
    chat_session_id: p.chatSessionId,
    start_time: String(Math.floor(p.startTime / 60)),
    over_time: String(Math.floor(p.overTime / 60)),
  });
  return request(`/message/history?${params.toString()}`);
}

// 最近 N 条消息（GET /message/recent，msg_count 必填，cur_time 可选且为 Unix 分钟）
// 注意：后端返回按 createTime 倒序（新→旧），展示前需排序
export function getRecent(p: {
  chatSessionId: string;
  msgCount: number;
  curTime?: number; // Unix 秒，可选
}): Promise<ApiRsp & { msgList?: MessageInfo[] }> {
  if (USE_MOCK) return mockGetRecent(p);
  const params = new URLSearchParams({
    chat_session_id: p.chatSessionId,
    msg_count: String(p.msgCount),
  });
  if (p.curTime !== undefined) {
    params.set("cur_time", String(Math.floor(p.curTime / 60)));
  }
  return request(`/message/recent?${params.toString()}`);
}

// 搜索会话内消息（GET /message/search）
export function searchMessages(p: {
  chatSessionId: string;
  searchKey: string;
}): Promise<ApiRsp & { msgList?: MessageInfo[] }> {
  if (USE_MOCK) return mockSearchMessages(p);
  const params = new URLSearchParams({
    chat_session_id: p.chatSessionId,
    search_key: p.searchKey,
  });
  return request(`/message/search?${params.toString()}`);
}

// 发送消息（POST /transmite/publish，body 为 proto3 JSON）
// 无服务端 messageId 返回，发送方靠本地临时 id 标识（AGENTS.md §6.6）
export function publishMessage(body: PublishReq): Promise<ApiRsp> {
  if (USE_MOCK) return mockPublishMessage();
  return request("/transmite/publish", {
    method: "POST",
    body: JSON.stringify(body),
  });
}
