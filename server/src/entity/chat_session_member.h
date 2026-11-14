//聊天会话成员表映射对象
#pragma once
#include <string>
#include <cstddef>
#include <odb/core.hxx>

namespace linkchat {
#pragma db object table("tb_chat_session_member")
class ChatSessionMember {
public:
    ChatSessionMember() {}
    ChatSessionMember(const std::string &ssid, const std::string &uid)
        : chat_session_id_(ssid), user_id_(uid) {}
    ~ChatSessionMember() {}

    const std::string &chat_session_id() const { return chat_session_id_; }
    void set_chat_session_id(const std::string &v) { chat_session_id_ = v; }

    const std::string &user_id() const { return user_id_; }
    void set_user_id(const std::string &v) { user_id_ = v; }

private:
    friend class odb::access;
    #pragma db id auto
    unsigned long id_;

    #pragma db type("varchar(64)") index
    std::string chat_session_id_; // 聊天 id，这里可能是单聊 id，也可能是群聊 id

    #pragma db type("varchar(64)")
    std::string user_id_; // 用户 id
};
}
