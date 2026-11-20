import type { ApiRsp, ChatSessionInfo, MessageInfo, UserInfo } from "@/types";

// Mock 数据与真实 api 同签名、同返回类型（AGENTS.md §3.1）
// M0 阶段所有请求走此文件，不接真实后端。

const T = 1763500000; // 消息时间基准（Unix 秒），仅用于演示时分

// 当前登录用户
export const CURRENT_USER_ID = "u1001";

export const me: UserInfo = {
  userId: "u1001",
  nickname: "我",
  description: "这个人很懒，什么都没写",
  email: "me@linkchat.local",
  avatar: "",
};

const zhangsan: UserInfo = {
  userId: "u1002",
  nickname: "张三",
  description: "前端工程师",
  email: "zhangsan@linkchat.local",
  avatar: "",
};

const lisi: UserInfo = {
  userId: "u1003",
  nickname: "李四",
  description: "后端工程师",
  email: "lisi@linkchat.local",
  avatar: "",
};

const wangwu: UserInfo = {
  userId: "u1004",
  nickname: "王五",
  description: "",
  email: "wangwu@linkchat.local",
  avatar: "",
};

function textMsg(
  messageId: string,
  chatSessionId: string,
  sender: UserInfo,
  content: string,
  createTime: number,
): MessageInfo {
  return {
    messageId,
    chatSessionId,
    sender,
    message: { messageType: "STRING", stringMessage: { content } },
    createTime: String(createTime),
  };
}

// 各会话的消息（按 createTime 升序）
const messagesBySession: Record<string, MessageInfo[]> = {
  s1: [
    textMsg("m101", "s1", zhangsan, "在吗？", T),
    textMsg("m102", "s1", me, "在的", T + 30),
    textMsg("m103", "s1", zhangsan, "周末有空吗", T + 90),
    textMsg("m104", "s1", me, "有空，啥事？", T + 480), // 距上条 390s > 5min，应产生时间分割线
    textMsg("m105", "s1", zhangsan, "一起吃个饭", T + 510),
  ],
  s2: [
    textMsg("m201", "s2", lisi, "文档我发你了", T + 100),
    textMsg("m202", "s2", me, "收到，我看看", T + 160),
    textMsg("m203", "s2", lisi, "好的", T + 200),
  ],
  s3: [
    textMsg("m301", "s3", zhangsan, "有人用过 react-virtuoso 吗", T + 300),
    textMsg("m302", "s3", lisi, "用过，长列表性能不错", T + 330),
    textMsg("m303", "s3", me, "我们项目也准备用", T + 360),
    textMsg("m304", "s3", zhangsan, "好的谢谢", T + 390),
    textMsg("m305", "s3", lisi, "记得处理 IME 组合事件", T + 800), // 距上条 410s > 5min
    textMsg("m306", "s3", me, "收到", T + 830),
  ],
  s4: [
    textMsg("m401", "s4", wangwu, "你好", T + 600),
    textMsg("m402", "s4", me, "你好，有什么事？", T + 620),
  ],
  s5: [], // 空会话：演示空消息态
};

// 会话列表（prevMessage 取该会话最后一条）
const sessions: ChatSessionInfo[] = [
  {
    chatSessionId: "s1",
    chatSessionName: "张三",
    singleChatFriendId: "u1002",
    avatar: "",
    prevMessage: messagesBySession.s1[messagesBySession.s1.length - 1],
  },
  {
    chatSessionId: "s2",
    chatSessionName: "李四",
    singleChatFriendId: "u1003",
    avatar: "",
    prevMessage: messagesBySession.s2[messagesBySession.s2.length - 1],
  },
  {
    chatSessionId: "s3",
    chatSessionName: "前端学习交流群",
    avatar: "",
    prevMessage: messagesBySession.s3[messagesBySession.s3.length - 1],
  },
  {
    chatSessionId: "s4",
    chatSessionName: "王五",
    singleChatFriendId: "u1004",
    avatar: "",
    prevMessage: messagesBySession.s4[messagesBySession.s4.length - 1],
  },
  {
    chatSessionId: "s5",
    chatSessionName: "新群聊（无消息）",
    avatar: "",
  },
];

const friends: UserInfo[] = [zhangsan, lisi, wangwu];

// ---- 与真实 api 同签名的 mock 函数 ----

export function mockGetProfile(): Promise<ApiRsp & { userInfo: UserInfo }> {
  return Promise.resolve({ success: true, errmsg: "", userInfo: me });
}

export function mockGetChatSessionList(): Promise<
  ApiRsp & { chatSessionInfoList: ChatSessionInfo[] }
> {
  return Promise.resolve({
    success: true,
    errmsg: "",
    chatSessionInfoList: sessions,
  });
}

export function mockGetFriendList(): Promise<
  ApiRsp & { friendList: UserInfo[] }
> {
  return Promise.resolve({ success: true, errmsg: "", friendList: friends });
}

export function mockGetRecent(p: {
  chatSessionId: string;
  msgCount: number;
  curTime?: number;
}): Promise<ApiRsp & { msgList: MessageInfo[] }> {
  const all = messagesBySession[p.chatSessionId] ?? [];
  const list = p.curTime
    ? all.filter((m) => Number(m.createTime) <= p.curTime!)
    : all;
  return Promise.resolve({
    success: true,
    errmsg: "",
    msgList: list.slice(-p.msgCount),
  });
}

export function mockGetHistory(p: {
  chatSessionId: string;
  startTime: number; // Unix 秒
  overTime: number; // Unix 秒
}): Promise<ApiRsp & { msgList: MessageInfo[] }> {
  const all = messagesBySession[p.chatSessionId] ?? [];
  return Promise.resolve({
    success: true,
    errmsg: "",
    msgList: all.filter(
      (m) =>
        Number(m.createTime) >= p.startTime &&
        Number(m.createTime) <= p.overTime,
    ),
  });
}

export function mockSearchMessages(p: {
  chatSessionId: string;
  searchKey: string;
}): Promise<ApiRsp & { msgList: MessageInfo[] }> {
  const all = messagesBySession[p.chatSessionId] ?? [];
  const key = p.searchKey.toLowerCase();
  return Promise.resolve({
    success: true,
    errmsg: "",
    msgList: all.filter((m) =>
      (m.message.stringMessage?.content ?? "").toLowerCase().includes(key),
    ),
  });
}

export function mockPublishMessage(): Promise<ApiRsp> {
  return Promise.resolve({ success: true, errmsg: "" });
}

export function mockSetUserAvatar(): Promise<ApiRsp> {
  return Promise.resolve({ success: true, errmsg: "" });
}

export function mockSetUserNickname(nickname: string): Promise<ApiRsp> {
  me.nickname = nickname;
  return Promise.resolve({ success: true, errmsg: "" });
}

export function mockSetUserDescription(description: string): Promise<ApiRsp> {
  me.description = description;
  return Promise.resolve({ success: true, errmsg: "" });
}

export function mockLogout(): Promise<ApiRsp> {
  return Promise.resolve({ success: true, errmsg: "" });
}
