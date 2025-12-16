#pragma once

#include <cstdio>
#include <cstdarg>
#include <mutex>
#include <unordered_map>
#include <string>

// 日志管理器：按 pid 写入不同文件，线程安全单例
class PidLogger {
public:
    // 获取单例实例
    static PidLogger& instance() {
        static PidLogger logger;
        return logger;
    }

    // 写日志方法
    void log(int pid, const char* format, ...) {
        // 保护 pid_map 的访问
        {
            std::lock_guard<std::mutex> global_lock(map_mutex);

            auto& entry = pid_map[pid];
            if (!entry.file) {
                std::string filename = "log_" + std::to_string(pid) + ".txt";
                entry.file = std::fopen(filename.c_str(), "a");
                if (!entry.file) {
                    perror(("Failed to open file for pid " + std::to_string(pid)).c_str());
                    return;
                }
            }
        }

        // 使用 pid 对应的锁保护写入
        {
            std::lock_guard<std::mutex> pid_lock(pid_map[pid].mutex);

            va_list args;
            va_start(args, format);
            std::vfprintf(pid_map[pid].file, format, args);
            std::fflush(pid_map[pid].file);
            va_end(args);
        }
    }

private:
    struct FileEntry {
        FILE* file = nullptr;
        std::mutex mutex;
    };

    std::unordered_map<int, FileEntry> pid_map;
    std::mutex map_mutex;

    // 私有构造和析构
    PidLogger() = default;

    ~PidLogger() {
        for (auto& [pid, entry] : pid_map) {
            if (entry.file) std::fclose(entry.file);
        }
    }

    // 禁止复制和赋值
    PidLogger(const PidLogger&) = delete;
    PidLogger& operator=(const PidLogger&) = delete;
};
