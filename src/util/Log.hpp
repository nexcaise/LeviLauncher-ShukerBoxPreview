#pragma once

#include <android/log.h>
#include <fmt/core.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <mutex>

#ifndef LOG_TAG
#define LOG_TAG "ShulkerboxPreview"
#endif

class Log {
public:
    template<typename... Args>
    static void Info(fmt::string_view fmtStr, Args&&... args) {
        Print(ANDROID_LOG_INFO, fmtStr, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Error(fmt::string_view fmtStr, Args&&... args) {
        Print(ANDROID_LOG_ERROR, fmtStr, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Debug(fmt::string_view fmtStr, Args&&... args) {
        Print(ANDROID_LOG_DEBUG, fmtStr, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Warn(fmt::string_view fmtStr, Args&&... args) {
        Print(ANDROID_LOG_WARN, fmtStr, std::forward<Args>(args)...);
    }

private:
    static inline std::mutex mtx;
    static constexpr const char* logPath = "/storage/emulated/0/games/org.levimc/ShulkerboxPreview/log.txt";

    template<typename... Args>
    static void Print(int prio, fmt::string_view fmtStr, Args&&... args) {
        std::string message = fmt::vformat(fmtStr, fmt::make_format_args(args...));

        __android_log_print(prio, LOG_TAG, "%s", message.c_str());

        std::lock_guard<std::mutex> lock(mtx);

        std::filesystem::create_directories("/storage/emulated/0/games/org.levimc/ShulkerboxPreview");

        std::ofstream file(logPath, std::ios::app);
        if (file.is_open()) {
            file << message << std::endl;
        }
    }
};