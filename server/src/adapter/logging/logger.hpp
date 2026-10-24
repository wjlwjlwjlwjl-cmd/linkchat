#pragma once

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

std::shared_ptr<spdlog::logger> logger;

#define LOGGER_TRACE(format, ...) logger->TRACE("[{}: {}] => " + std::string(format), __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_DEBUG(format, ...) logger->debug("[{}: {}] => " + std::string(format), __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_INFO(format, ...) logger->info("[{}: {}] => " + std::string(format), __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_WARN(format, ...) logger->warn("[{}: {}] => " + std::string(format), __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_ERROR(format, ...) logger->error("[{}: {}] => " + std::string(format), __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGGER_CRITICAL(format, ...) logger->critical("[{}: {}] => " + std::string(format), __FILE__, __LINE__, ##__VA_ARGS__)

/**
 *
 * @param durable 是否开启持久化，即输出到stdout还是到文件持久化
 * @param pattern 日志格式
 * @param level 输出级别（默认info）
 * @param log_file 日志文件
 */
void init_logger(bool durable = false, const std::string& pattern = "%^%L%$ %v", spdlog::level::level_enum level = spdlog::level::info, const std::string& log_file = "") {
    spdlog::init_thread_pool(std::thread::hardware_concurrency(), 1);
    if (!durable) {
        //落地方向为标准输出的日志器件
        logger = spdlog::stdout_color_mt<spdlog::async_factory>("g_async_logger", spdlog::color_mode::always);
        logger->set_pattern(pattern);
        logger->set_level(level);
    }
    else {
        //落地方向为文件的日志器
        logger = spdlog::basic_logger_mt<spdlog::async_factory>("g_async_logger", log_file);
        logger->set_pattern(pattern);
        logger->set_level(level);
    }
}