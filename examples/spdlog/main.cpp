#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

void multi_sink_logger_test() {
    //不同落地方向的落地对象
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("spdlog.txt", true);

    spdlog::logger logger = spdlog::logger("multi_sink_logger", {stdout_sink, file_sink});
    logger.set_pattern("%H:%M:%S.%e %^%L%$ %v");
    logger.set_level(spdlog::level::info);

    logger.info("Hello World!");
    logger.warn("Hello World!");
    logger.error("Hello World!");
    logger.critical("Hello World!");
}

void async_logger_test() {
    spdlog::init_thread_pool(std::thread::hardware_concurrency(), 1);
    auto logger = spdlog::stdout_color_mt<spdlog::async_factory>("async_logger", spdlog::color_mode::always);
    logger->set_pattern("%H:%M:%S.%e %^%L%$ %v");
    logger->set_level(spdlog::level::info);

    logger->info("Hello World!");
    logger->warn("Hello World!");
    logger->error("Hello World!");
    logger->critical("Hello World!");
}

int main() {
    //multi_sink_logger_test();
    async_logger_test();
    return 0;
}