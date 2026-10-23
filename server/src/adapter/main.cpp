#include "logging/spdlog_logger.hpp"

int main() {
    init_logger(false);
    LOGGER_INFO("test logger info: {}", "hello world");
    LOGGER_WARN("test logger warn: {}", "hello world");
    LOGGER_ERROR("test logger error: {}", "hello world");
    return 0;
}