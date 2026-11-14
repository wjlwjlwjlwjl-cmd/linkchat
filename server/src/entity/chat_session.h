#pragma once
#include "chat_session_member.h"
#include <cstddef>
#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <string>

namespace linkchat {

enum class ChatSessionType { SINGLE = 1, GROUP = 2 };

#pragma db object table("tb_chat_session")
class ChatSession {
public:
    ChatSession() {}
    ChatSession(const std::string &ssid, const std::string &ssname,
                const ChatSessionType sstype)
        : chat_session_id_(ssid), chat_session_name_(ssname),
          chat_session_type_(sstype) {}

    const std::string &chat_session_id() const { return chat_session_id_; }
    void set_chat_session_id(const std::string &v) { chat_session_id_ = v; }

    const std::string &chat_session_name() const { return chat_session_name_; }
    void set_chat_session_name(const std::string &v) { chat_session_name_ = v; }

    ChatSessionType chat_session_type() const { return chat_session_type_; }
    void set_chat_session_type(ChatSessionType v) { chat_session_type_ = v; }

private:
    friend class odb::access;

    #pragma db id auto
    unsigned long id_;

    #pragma db type("varchar(64)") index unique
    std::string chat_session_id_;

    #pragma db type("varchar(64)")
    std::string chat_session_name_;

    #pragma db type("tinyint")
    ChatSessionType chat_session_type_; // 1-单聊； 2-群聊
};

// 将多表查询 join 的结果，映射到只读的结构体中

// 处理单聊，直接获取当前用户所有单聊对话的会话id和对方id
// 连接 ChatSession 表和 ChatSessionMember 表，通过 csm1 过滤出所需会话，通过 csm2 和 csm1 排除自己
// 这里条件必须是指定条件：  css::chat_session_type==1 && csm1.user_id=uid && csm2.user_id != csm1.user_id
#pragma db view object(ChatSession = css) object(                              \
    ChatSessionMember = csm1 : css::chat_session_id_ == csm1::chat_session_id_)     \
    object(ChatSessionMember =                                                 \
               csm2 : css::chat_session_id_ == csm2::chat_session_id_) query((?))
struct SingleChatSession {
#pragma db column(css::chat_session_id_)
    std::string chat_session_id;
#pragma db column(csm2::user_id_)
    std::string friend_id;
};
// 处理群聊，直接获取当前用户所有群聊对话的会话id和群名
// 这里条件必须是指定条件：  css::chat_session_type==2 && csm.user_id=uid
#pragma db view object(ChatSession = css)                                      \
    object(ChatSessionMember =                                                 \
               csm : css::chat_session_id_ == csm::chat_session_id_) query((?))
struct GroupChatSession {
#pragma db column(css::chat_session_id_)
    std::string chat_session_id;
#pragma db column(css::chat_session_name_)
    std::string chat_session_name;
};

} // namespace linkchat
