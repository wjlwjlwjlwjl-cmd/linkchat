#pragma once

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

#include "util/string_util.hpp"

std::shared_ptr<spdlog::logger> logger;

const char* get_current_filename() {
    // 从整个路径中，从后往前找最后一个 '/' 或 '\'
    const char* file = __FILE__;
    const char* p1 = strrchr(file, '/');
    const char* p2 = strrchr(file, '\\');
    const char* p = p1 > p2 ? p1 : p2; // 取位置更靠后的那个（兼容不同系统）
    
    return p ? p + 1 : file;
}

std::string cur_f(get_current_filename());

#define TRC(format, ...) logger->trace("[{}: {}] => " format, cur_f, __LINE__, ##__VA_ARGS__)
#define DBG(format, ...) logger->debug("[{}: {}] => " format, cur_f, __LINE__, ##__VA_ARGS__)
#define INF(format, ...) logger->info("[{}: {}] => " format, cur_f, __LINE__, ##__VA_ARGS__)
#define WRN(format, ...) logger->warn("[{}: {}] => " format, cur_f, __LINE__, ##__VA_ARGS__)
#define ERR(format, ...) logger->error("[{}: {}] => " format, cur_f, __LINE__, ##__VA_ARGS__)
#define CRT(format, ...) logger->critical("[{}: {}] => " format, cur_f, __LINE__, ##__VA_ARGS__)

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