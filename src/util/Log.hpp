#pragma once

#include <android/log.h>
#include <format>
#include <string>

#ifndef LOG_TAG
#define LOG_TAG "ShulkerboxPreview"
#endif

class Log {
public:
    template<typename... Args>
    static void Info(std::format_string<Args...> fmt, Args&&... args) {
        Print(ANDROID_LOG_INFO, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Error(std::format_string<Args...> fmt, Args&&... args) {
        Print(ANDROID_LOG_ERROR, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Debug(std::format_string<Args...> fmt, Args&&... args) {
        Print(ANDROID_LOG_DEBUG, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Warn(std::format_string<Args...> fmt, Args&&... args) {
        Print(ANDROID_LOG_WARN, fmt, std::forward<Args>(args)...);
    }

private:
    template<typename... Args>
    static void Print(int prio, std::format_string<Args...> fmt, Args&&... args) {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        __android_log_print(prio, LOG_TAG, "%s", message.c_str());
    }
};