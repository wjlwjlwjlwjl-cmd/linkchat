#pragma once

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

#include "string_util.h"

// 定义在头文件中的全局对象用 inline 修饰，保证整个程序只存在一份实体
inline std::shared_ptr<spdlog::logger> g_logger;

inline const char* GetCurrentFilename() {
    // 从整个路径中，从后往前找最后一个 '/' 或 '\'
    const char* file = __FILE__;
    const char* p1 = strrchr(file, '/');
    const char* p2 = strrchr(file, '\\');
    const char* p = p1 > p2 ? p1 : p2; // 取位置更靠后的那个（兼容不同系统）

    return p ? p + 1 : file;
}

inline std::string g_cur_f(GetCurrentFilename());

#define LOG_TRACE(format, ...) g_logger->trace("[{}: {}] => " format, g_cur_f, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LOG_DEBUG(format, ...) g_logger->debug("[{}: {}] => " format, g_cur_f, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LOG_INFO(format, ...) g_logger->info("[{}: {}] => " format, g_cur_f, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LOG_WARN(format, ...) g_logger->warn("[{}: {}] => " format, g_cur_f, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LOG_ERROR(format, ...) g_logger->error("[{}: {}] => " format, g_cur_f, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LOG_CRITICAL(format, ...) g_logger->critical("[{}: {}] => " format, g_cur_f, __LINE__ __VA_OPT__(,) __VA_ARGS__)

/**
 *
 * @param durable 是否开启持久化，即输出到stdout还是到文件持久化
 * @param pattern 日志格式
 * @param level 输出级别（默认info）
 * @param log_file 日志文件
 */
inline void InitLogger(bool durable = false, const std::string& pattern = "%^%L%$ %v",
                       spdlog::level::level_enum level = spdlog::level::info, const std::string& log_file = "") {
    spdlog::init_thread_pool(std::thread::hardware_concurrency(), 1);
    if (!durable) {
        //落地方向为标准输出的日志器件
        g_logger = spdlog::stdout_color_mt<spdlog::async_factory>("g_async_logger", spdlog::color_mode::always);
        g_logger->set_pattern(pattern);
        g_logger->set_level(level);
    }
    else {
        //落地方向为文件的日志器
        g_logger = spdlog::basic_logger_mt<spdlog::async_factory>("g_async_logger", log_file);
        g_logger->set_pattern(pattern);
        g_logger->set_level(level);
    }
}
