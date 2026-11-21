// LinkChat 前端类型定义（对应后端 proto3 JSON）
// proto3 JSON 会省略默认值字段：int64 为 string，空串/0/false 字段可能整个缺失，
// 因此除主键外一律声明为可选，读取时必须兜底。

// 用户信息（base.proto UserInfo）
// 实测：消息里的 sender 只返回 userId，其余字段缺失
export interface UserInfo {
  userId: string;
  nickname?: string;
  description?: string;
  email?: string;
  avatar?: string; // file_id（如 file_4bf3bd13），不是 URL
}

// 消息类型（proto3 JSON 序列化为字符串；STRING 为默认值 0，会被省略）
export type MessageType = "STRING" | "FILE";

export interface StringMessageInfo {
  content: string;
}

export interface FileMessageInfo {
  fileId?: string;
  fileSize?: string; // int64 → string
  fileName?: string;
  fileContent?: string; // bytes → base64
}

export interface MessageContent {
  messageType?: MessageType; // 缺失时按 STRING 处理
  stringMessage?: StringMessageInfo;
  fileMessage?: FileMessageInfo;
}

// 消息结构（base.proto MessageInfo）
export interface MessageInfo {
  messageId: string;
  chatSessionId: string;
  sender: UserInfo;
  message: MessageContent;
  createTime: string; // int64（Unix 秒）→ string
}

// 聊天会话（base.proto ChatSessionInfo）
export interface ChatSessionInfo {
  singleChatFriendId?: string;
  chatSessionId: string;
  chatSessionName: string;
  prevMessage?: MessageInfo;
  avatar?: string; // file_id
}

// 好友申请事件（friend.proto FriendEvent）
export interface FriendEvent {
  eventId: string;
  sender: UserInfo;
}

// 文件下载数据（base.proto FileDownloadData）
export interface FileDownloadData {
  fileId: string;
  presignedUrl: string;
}

// 统一响应包装（success=false / errmsg="" 时字段可能缺失）
export interface ApiRsp {
  success?: boolean;
  errmsg?: string;
}

// 本地消息状态（AGENTS.md §6.6）：发送中 / 已发送 / 发送失败
export type MessageStatus = "sending" | "sent" | "failed";

// 本地消息：服务端 MessageInfo + 发送状态（临时 id 以 temp- 开头）
export interface LocalMessage extends MessageInfo {
  status: MessageStatus;
  // 发送失败的原因（如被对方删除），展示在失败消息左侧
  failReason?: string;
}

// 发送消息请求体（POST /transmite/publish，body 为 proto3 JSON，字段 camelCase）
export interface PublishReq {
  chatSessionId: string;
  message: {
    messageType: MessageType;
    stringMessage?: StringMessageInfo;
    fileMessage?: FileMessageInfo;
  };
}

// WS 推送类型（AGENTS.md §6.7）
export type MsgType =
  | "NEW_MESSAGE"
  | "FRIEND_REQUEST"
  | "FRIEND_APPLY_RESULT"
  | "CHAT_SESSION_CREATED";

// WS 推送信封：{"type": "...", "data": {...}}
export interface WsEnvelope<T = unknown> {
  type: MsgType;
  data: T;
}

export interface FriendRequestData {
  notifyEventId?: string;
  applyUserId: string;
}

export interface FriendApplyResultData {
  agree?: boolean;
  newSessionId?: string;
}

export interface ChatSessionCreatedData {
  chatSessionId: string;
  chatSessionName?: string;
  creatorId?: string;
}
