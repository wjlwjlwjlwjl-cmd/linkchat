#pragma once

// 各服务的错误提示文案统一在此维护，避免在 dao / service / controller 中散落硬编码。
// 按服务划分命名空间；多个服务共用的文案放在 errmsg 顶层。
namespace errmsg {

// ---------- 多服务共用 ----------
inline constexpr const char* kTokenInvalid = "token 无效";
inline constexpr const char* kUserNotFound = "用户不存在";

// ---------- 网关服务 ----------
namespace gateway{
    inline constexpr const char* kWebSocketFailed = "WebSocket 连接已失效，请重新连接";
}

// ---------- 用户服务 ----------
namespace user {
inline constexpr const char* kVerifyCodeIncorrect = "验证码错误";
inline constexpr const char* kSendVerifyCodeFailed = "验证码邮件发送失败";
inline constexpr const char* kTryAgainLater = "请稍后重试";
inline constexpr const char* kInitChannelFailed = "服务连接初始化失败";
inline constexpr const char* kPutFileFailed = "上传文件失败";
} // namespace user

// ---------- 好友服务 ----------
namespace friend_service {
// service 层
inline constexpr const char* kFriendListFailed = "获取好友列表失败";
inline constexpr const char* kFriendRemoveFailed = "删除好友失败";
inline constexpr const char* kFriendAddProcessFailed = "处理好友申请失败";
inline constexpr const char* kPendingEventListFailed = "获取待处理好友申请列表失败";
inline constexpr const char* kFriendSearchFailed = "搜索用户失败";
inline constexpr const char* kChatSessionListFailed = "获取会话列表失败";
inline constexpr const char* kChatSessionCreateFailed = "创建会话失败";
inline constexpr const char* kChatSessionMemberFailed = "获取会话成员失败";

// dao 层
inline constexpr const char* kRelationNotFound = "好友关系不存在";
inline constexpr const char* kFriendNotFound = "好友不存在";
inline constexpr const char* kAlreadyFriend = "已经是好友关系";
inline constexpr const char* kFriendApplyNotFound = "好友申请不存在";
inline constexpr const char* kGroupAlreadyExists = "已存在同名且成员相同的群聊";
} // namespace friend_service

// ---------- 文件服务 ----------
namespace file {
inline constexpr const char* kFileIdEmpty = "file_id 不能为空";
inline constexpr const char* kGetPresignedUrlFailed = "获取预签名链接失败";
inline constexpr const char* kFileNameEmpty = "file_name 不能为空";
inline constexpr const char* kUploadFailed = "上传文件失败";
} // namespace file

// ---------- 消息服务 ----------
namespace message {
inline constexpr const char* kSearchParamEmpty = "chat_session_id 和 search_key 不能为空";
} // namespace message

// ---------- 消息转发服务 ----------
namespace transmite {
inline constexpr const char* kPublishToMqFailed = "消息发送到消息队列失败";
inline constexpr const char* kChatSessionNotFound = "会话不存在或成员信息异常";
inline constexpr const char* kNotFriend = "你还不是他（她）朋友。请先发送朋友验证请求，对方验证通过后，才能聊天。";
inline constexpr const char* kNotGroupMember = "你不是该群聊成员，无法发送消息";
} // namespace transmite

} // namespace errmsg
