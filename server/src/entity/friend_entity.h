// 好友申请类，用于存储好友申请信息

#pragma once
#include <cstddef>
#include <odb/core.hxx>
#include <string>

namespace linkchat {
#pragma db object table("tb_friend")
class FriendApply {
public:
    FriendApply() {}
    FriendApply(const std::string &eid, const std::string &uid,
                const std::string &pid)
        : user_id_(uid), peer_id_(pid), event_id_(eid) {}

    const std::string &event_id() const { return event_id_; }
    void set_event_id(const std::string &v) { event_id_ = v; }

    const std::string &user_id() const { return user_id_; }
    void set_user_id(const std::string &v) { user_id_ = v; }

    const std::string &peer_id() const { return peer_id_; }
    void set_peer_id(const std::string &v) { peer_id_ = v; }

private:
    friend class odb::access;
#pragma db id auto
    unsigned long id_;
#pragma db type("varchar(64)") index unique
    std::string event_id_;
#pragma db type("varchar(64)") index
    std::string user_id_;
#pragma db type("varchar(64)") index
    std::string peer_id_;
};
} // namespace linkchat
