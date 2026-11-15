#include <vector>

#include "common_dto.h"

struct ChatSessionCreateDTO: public CommonDTO {
    std::string chat_session_id;
};