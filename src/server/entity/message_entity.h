#pragma once

#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <odb/section.hxx>
#include <string>
#include <stddef.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#pragma db object table("tb_msg")
class MessageEntity {
public:
    MessageEntity() {}

    unsigned long long id() const { return id_; }

    const std::string& message_id() const { return message_id_; }
    void set_message_id(const std::string& v) { message_id_ = v; }

    const boost::posix_time::ptime& create_time() const { return create_time_; }
    void set_create_time(const boost::posix_time::ptime& v) { create_time_ = v; }

    const std::string& from_user_id() const { return from_user_id_; }
    void set_from_user_id(const std::string& v) { from_user_id_ = v; }

    const std::string& to_session_id() const { return to_session_id_; }
    void set_to_session_id(const std::string& v) { to_session_id_ = v; }

    signed char message_type() const { return message_type_; }
    void set_message_type(signed char v) { message_type_ = v; }

    const odb::nullable<std::string>& content() const { return content_; }
    void set_content(const odb::nullable<std::string>& v) { content_ = v; }
    void set_content(const std::string& v) { content_ = v; }
    void clear_content() { content_ = odb::nullable<std::string>(); }
    bool has_content() const { return !content_.null(); }

    const odb::nullable<std::string>& file_id() const { return file_id_; }
    void set_file_id(const odb::nullable<std::string>& v) { file_id_ = v; }
    void set_file_id(const std::string& v) { file_id_ = v; }
    void clear_file_id() { file_id_ = odb::nullable<std::string>(); }
    bool has_file_id() const { return !file_id_.null(); }

    const odb::nullable<std::string>& file_name() const { return file_name_; }
    void set_file_name(const odb::nullable<std::string>& v) { file_name_ = v; }
    void set_file_name(const std::string& v) { file_name_ = v; }
    void clear_file_name() { file_name_ = odb::nullable<std::string>(); }
    bool has_file_name() const { return !file_name_.null(); }

    const odb::nullable<unsigned long long>& file_size() const { return file_size_; }
    void set_file_size(const odb::nullable<unsigned long long>& v) { file_size_ = v; }
    void set_file_size(unsigned long long v) { file_size_ = v; }
    void clear_file_size() { file_size_ = odb::nullable<unsigned long long>(); }
    bool has_file_size() const { return !file_size_.null(); }

    //用来获取 setion 标记为已变更，后续update就可以让 odb 生成部分更新的代码
    odb::section& file_metadata_section(){
        return file_metadata_;
    }

private:
    friend class odb::access;

    #pragma db id auto
    unsigned long long id_; // 自增主键

    #pragma db unique type("VARCHAR(127)")
    std::string message_id_; // 消息 id

    #pragma db type("TIMESTAMP") not_null
    boost::posix_time::ptime create_time_; //创建时间

    #pragma db type("VARCHAR(127)")
    std::string from_user_id_; // 发送者 id

    #pragma db type("VARCHAR(127)")
    std::string to_session_id_; // 所属对话 id

    #pragma db type("TINYINT")
    signed char message_type_; // 消息类型

    #pragma db type("TEXT")
    odb::nullable<std::string> content_; //消息内容（文字类型)

    #pragma db update(change)
    odb::section file_metadata_;

    #pragma db section(file_metadata_)
    #pragma db type("VARCHAR(127)")
    odb::nullable<std::string> file_id_;

    #pragma db section(file_metadata_)
    #pragma db type("VARCHAR(127)")
    odb::nullable<std::string> file_name_;

    #pragma db section(file_metadata_)
    #pragma db type("BIGINT UNSIGNED")
    odb::nullable<unsigned long long> file_size_;
};