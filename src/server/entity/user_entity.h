#pragma once
#include <string>
#include <odb/nullable.hxx>

#pragma db object table("tb_user")
namespace linkchat{
    class User {
        public:
            User() = default;

            User(const std::string &user_id,
                const odb::nullable<std::string> &nickname,
                const odb::nullable<std::string> &description,
                const odb::nullable<std::string> &password,
                const odb::nullable<std::string> &email,
                const odb::nullable<std::string> &avatar_id)
                : _user_id(user_id),
                _nickname(nickname),
                _description(description),
                _password(password),
                _email(email),
                _avatar_id(avatar_id) {}

            unsigned long id() const { return _id; }
            void set_id(unsigned long id) { _id = id; }

            const std::string &user_id() const { return _user_id; }
            void set_user_id(const std::string &user_id) { _user_id = user_id; }

            const odb::nullable<std::string> &nickname() const { return _nickname; }
            void set_nickname(const odb::nullable<std::string> &nickname) { _nickname = nickname; }
            void set_nickname(const std::string &nickname) { _nickname = nickname; }
            void clear_nickname() { _nickname = odb::nullable<std::string>(); }
            bool has_nickname() const { return !_nickname.null(); }

            const odb::nullable<std::string> &description() const { return _description; }
            void set_description(const odb::nullable<std::string> &description) { _description = description; }
            void set_description(const std::string &description) { _description = description; }
            void clear_description() { _description = odb::nullable<std::string>(); }
            bool has_description() const { return !_description.null(); }

            const odb::nullable<std::string> &password() const { return _password; }
            void set_password(const odb::nullable<std::string> &password) { _password = password; }
            void set_password(const std::string &password) { _password = password; }
            void clear_password() { _password = odb::nullable<std::string>(); }
            bool has_password() const { return !_password.null(); }

            const odb::nullable<std::string> &email() const { return _email; }
            void set_email(const odb::nullable<std::string> &email) { _email = email; }
            void set_email(const std::string &email) { _email = email; }
            void clear_email() { _email = odb::nullable<std::string>(); }
            bool has_email() const { return !_email.null(); }

            const odb::nullable<std::string> &avatar_id() const { return _avatar_id; }
            void set_avatar_id(const odb::nullable<std::string> &avatar_id) { _avatar_id = avatar_id; }
            void set_avatar_id(const std::string &avatar_id) { _avatar_id = avatar_id; }
            void clear_avatar_id() { _avatar_id = odb::nullable<std::string>(); }
            bool has_avatar_id() const { return !_avatar_id.null(); }

        private:
            friend class odb::access;

            #pragma db id auto
            unsigned long _id;

            #pragma db type("varchar(64)") index unique
            std::string _user_id;

            #pragma db type("varchar(64)") index unique
            odb::nullable<std::string> _nickname; //用户昵称

            odb::nullable<std::string> _description; //用户签名

            #pragma db type("varchar(255)")
            odb::nullable<std::string> _password; //用户密码

            #pragma db type("varchar(64)") index unique
            odb::nullable<std::string> _email; //用户邮箱

            #pragma db type("varchar(64)")
            odb::nullable<std::string> _avatar_id; //用户头像文件ID，即文件存储服务的 file_id，需要时填充为预签名连接即可
    };
}