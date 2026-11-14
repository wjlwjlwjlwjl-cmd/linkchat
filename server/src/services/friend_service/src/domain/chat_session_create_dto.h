#include <vector>

#include "common_dto.h"

struct ChatSessionCreateDTO: public CommonDTO {
    std::string chat_session_id;
    std::string chat_session_name;
    std::vector<std::string> member_id_list;
};