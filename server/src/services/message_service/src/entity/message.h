#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <string>
#include <stddef.h>
#include <boost/date_time/posix_time/posix_time.hpp>

class Message{
public:
    Message(){}
private:
    friend class odb::access;

    #pragma db id auto 
    unsigned long long id_;

    #pragma db unique type("VARCHAR(127)")
    std::string message_id_;

    #pragma db type("TIMESTAMP") not_null
    boost::posix_time::ptime create_time_;

    #pragma db type("VARCHAR(127)")
    std::string from_user_id_;

    #pragma db type("VARCHAR(127)")
    std::string to_session_id_;

    #pragma db type("TINYINT")
    signed char message_type_;

    odb::nullable<std::string> content_;

    #pragma db type("VARCHAR(127)")
    odb::nullable<std::string> file_id_;

    #pragma db type("VARCHAR(127)")
    odb::nullable<std::string> file_name_;

    #pragma db type("BIGINT UNSIGNED")
    odb::nullable<unsigned long long> file_size_;
};