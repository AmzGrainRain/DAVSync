#pragma once
#define LOG_LEVEL_INFO

#if defined(LOG_LEVEL_INFO)
#define LOG_INFO(format_str)                                                                                           \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] INFO: ";                                                                    \
    std::cerr << format_str << std::endl;
#define LOG_INFO_FMT(format_str, ...)                                                                                  \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] INFO: ";                                                                    \
    std::cerr << std::format(format_str, ##__VA_ARGS__) << std::endl;
#define LOG_WARN(format_str, ...)                                                                                      \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] WARN: ";                                                                    \
    std::cerr << format_str << std::endl;
#define LOG_WARN_FMT(format_str, ...)                                                                                  \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] WARN: ";                                                                    \
    std::cerr << std::format(format_str, ##__VA_ARGS__) << std::endl;
#define LOG_ERROR(format_str, ...)                                                                                     \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] ERROR: ";                                                                   \
    std::cerr << format_str << std::endl;
#define LOG_ERROR_FMT(format_str, ...)                                                                                 \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] ERROR: ";                                                                   \
    std::cerr << std::format(format_str, ##__VA_ARGS__) << std::endl;

#elif define(LOG_LEVEL_WARN)
#define LOG_INFO(format_str)
#define LOG_INFO_FMT(format_str, ...)
#define LOG_WARN(format_str, ...)                                                                                      \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] WARN: ";                                                                    \
    std::cerr << format_str << std::endl;
#define LOG_WARN_FMT(format_str, ...)                                                                                  \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] WARN: ";                                                                    \
    std::cerr << std::format(format_str, ##__VA_ARGS__) << std::endl;
#define LOG_ERROR(format_str, ...)                                                                                     \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] ERROR: ";                                                                   \
    std::cerr << format_str << std::endl;
#define LOG_ERROR_FMT(format_str, ...)                                                                                 \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] ERROR: ";                                                                   \
    std::cerr << std::format(format_str, ##__VA_ARGS__) << std::endl;

#elif define(LOG_LEVEL_ERROR)
#define LOG_INFO(format_str)
#define LOG_INFO_FMT(format_str, ...)
#define LOG_WARN(format_str, ...)
#define LOG_WARN_FMT(format_str, ...)
#define LOG_ERROR(format_str, ...)                                                                                     \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] ERROR: ";                                                                   \
    std::cerr << format_str << std::endl;
#define LOG_ERROR_FMT(format_str, ...)                                                                                 \
    std::cerr << '[' << __FILE__ << ':' << __LINE__ << ']';                                                            \
    std::cerr << '[' << __FUNCTION__ << "] ERROR: ";                                                                   \
    std::cerr << std::format(format_str, ##__VA_ARGS__) << std::endl;
#endif
