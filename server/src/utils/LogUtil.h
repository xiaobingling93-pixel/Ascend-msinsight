/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: LogUtil Utility declaration
 */

#ifndef DATA_INSIGHT_CORE_LOG_H
#define DATA_INSIGHT_CORE_LOG_H

#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <mutex>
#include "TimeUtil.h"

namespace Dic {
enum class LogLevel : int {
    L_DEBUG = 0,
    L_INFO,
    L_WARN,
    L_ERROR,
    L_FATAL
};

enum class LogOutType : int {
    TERMINAL = 0,
    FILE,
    BOTH
};

class LogPrefix {
public:
    static LogPrefix &Instance()
    {
        static LogPrefix instance;
        return instance;
    }

    inline const std::string TimePrefix(const TimeStyle &style = TimeStyle::WITH_MILLI_SEC) const
    {
        return TimeUtil::Instance().NowStr(style);
    }

    inline const std::string LevelPrefix(const LogLevel &level) const
    {
        static std::map<LogLevel, std::string> map = { { LogLevel::L_INFO, "[Info]" },
                                                       { LogLevel::L_WARN, "[Warn]" },
                                                       { LogLevel::L_ERROR, "[Error]" },
                                                       { LogLevel::L_FATAL, "[Fatal]" },
                                                       { LogLevel::L_DEBUG, "[Debug]" } };
        if (map.count(level) == 0) {
            return map[LogLevel::L_INFO];
        }
        return map[level];
    }

    inline const std::string LocationPrefix(const std::string &file, const std::string &func, const int &line) const
    {
        std::stringstream ss;
        ss << " (" << file << ", " << func << ", line: " << line << ") |";
        return ss.str();
    }

private:
    LogPrefix() = default;
    ~LogPrefix() = default;
};

class LogUtil {
public:
    LogUtil(const LogOutType &type, const std::string &filePath, const std::string &wsPort)
        : outType(type), originFilePath(filePath), wsPort(wsPort)
    {
        Initialize();
    }

    ~LogUtil()
    {
        Destroy();
    }

    static std::unique_ptr<LogUtil> NewInstance(const LogOutType &type, const std::string &logFilePath,
                                                const std::string &wsPort)
    {
        return std::make_unique<LogUtil>(type, logFilePath, wsPort);
    }

    inline LogUtil &SetOutType(const LogOutType &type)
    {
        if (this->outType != type) {
            this->outType = type;
        }
        return *this;
    }

    inline LogUtil &SetFilePath(const std::string &logFilePath)
    {
        if (this->filePath != logFilePath) {
            this->filePath = logFilePath;
        }
        return *this;
    }

    inline LogUtil &SetLogLevel(const LogLevel &logLevel)
    {
        if (this->level != logLevel) {
            this->level = logLevel;
        }
        return *this;
    }

    inline LogUtil &SetWsPort(const std::string &port)
    {
        this->wsPort = port;
        return *this;
    }

    inline LogUtil &SetMaxSize(const int logMaxSize)
    {
        if (currentSize > logMaxSize) {
            this->ofs.close();
            this->ofs.open(filePath, std::ofstream::out | std::ofstream::trunc);
            isAppend = false;
            currentSize = 0;
        }
        if (this->maxSize != logMaxSize) {
            this->maxSize = logMaxSize;
        }
        return *this;
    }

    template <typename... ARGS> inline LogUtil &Info(const ARGS... args)
    {
        LogT(LogLevel::L_INFO, LogPrefix::Instance().LevelPrefix(LogLevel::L_INFO), args...);
        return *this;
    }

    template <typename... ARGS> inline LogUtil &Warn(const ARGS... args)
    {
        LogT(LogLevel::L_WARN, LogPrefix::Instance().LevelPrefix(LogLevel::L_WARN), args...);
        return *this;
    }

    template <typename... ARGS> inline LogUtil &Error(const ARGS... args)
    {
        LogT(LogLevel::L_ERROR, LogPrefix::Instance().LevelPrefix(LogLevel::L_ERROR), args...);
        return *this;
    }

    template <typename... ARGS> inline LogUtil &Fatal(const ARGS... args)
    {
        LogT(LogLevel::L_FATAL, LogPrefix::Instance().LevelPrefix(LogLevel::L_FATAL), args...);
        return *this;
    }

    template <typename... ARGS> inline LogUtil &Debug(const ARGS... args)
    {
        LogT(LogLevel::L_DEBUG, LogPrefix::Instance().LevelPrefix(LogLevel::L_DEBUG), args...);
        return *this;
    }

    template <typename... ARGS> inline LogUtil &operator << (const ARGS... args)
    {
        LogT(args...);
        return *this;
    }

    template <typename T, typename... ARGS> inline void LogT(const LogLevel &logLevel, const T &t, const ARGS... args)
    {
        if (logLevel < level) {
            return;
        }
        std::string str = GetString(t, args...);
        LogStr(str);
    }

    static LogLevel GetLogLevel(const std::string &logLevel)
    {
        static std::map<std::string, LogLevel> map = { { "INFO", LogLevel::L_INFO },
                                                       { "WARN", LogLevel::L_WARN },
                                                       { "ERROR", LogLevel::L_ERROR },
                                                       { "FATAL", LogLevel::L_FATAL },
                                                       { "DEBUG", LogLevel::L_DEBUG } };
        if (map.count(logLevel) != 0) {
            return map.at(logLevel);
        }
        return LogLevel::L_INFO;
    }

private:
    void Initialize()
    {
        filePath = originFilePath;
        std::string::size_type pos = filePath.find_last_of(".");
        std::string insertPort;
        if (!wsPort.empty() && wsPort != "-1") {
            insertPort += "_" + wsPort;
        }
        if (pos != std::string::npos) {
            filePath.insert(pos, insertPort);
            filePath.insert(pos + insertPort.size(), "_" + std::to_string(++count));
        } else {
            filePath += insertPort;
            filePath += ("_" + std::to_string(++count));
        }
        if ((this->outType != LogOutType::TERMINAL) && !this->filePath.empty()) {
            currentSize = GetFileSize(filePath);
            if (currentSize > maxSize) {
                isAppend = false;
                this->ofs.open(filePath, std::ofstream::out | std::ofstream::trunc);
                currentSize = 0;
            } else {
                this->ofs.open(filePath, std::ofstream::out | std::ofstream::app);
            }
        }
    }

    void Destroy()
    {
        if (ofs.is_open()) {
            ofs.close();
        }
    }

    template <typename T> inline std::string GetString(const T &t) const
    {
        std::stringstream ss;
        ss << t;
        return ss.str();
    }

    template <typename T, typename... ARGS> inline std::string GetString(const T &t, const ARGS... args) const
    {
        std::string str = GetString(t);
        str += GetString(args...);
        return str;
    }

    inline int GetFileSize(const std::string &path) const
    {
        std::ifstream fr;
        fr.open(path, std::ios::in | std::ios::binary);
        int result = 0;
        if (fr.is_open()) {
            fr.seekg(0, fr.end);
            result = static_cast<int>(fr.tellg());
        }
        fr.close();
        return result;
    }

    inline void Check()
    {
        CheckPort();
        if ((currentSize >= maxSize) && ofs.is_open()) {
            ofs.close();
            filePath = originFilePath;
            if (count >= maxCount) {
                count = 0;
                isAppend = false;
            }
            filePath.insert(filePath.find_last_of("."), "_" + std::to_string(++count));
            if (isAppend) {
                currentSize = GetFileSize(filePath);
                ofs.open(filePath, std::ios::out | std::ios::app);
            } else {
                ofs.open(filePath, std::ios::out | std::ios::trunc);
                currentSize = 0;
            }
        }
    }

    inline void CheckPort()
    {
        if (!wsPort.empty() && wsPort != "-1" && ofs.is_open()) {
            ofs.close();
            filePath = originFilePath;
            filePath.insert(filePath.find_last_of("."), "_" + wsPort);
            filePath.insert(filePath.find_last_of("."), "_" + std::to_string(count));
            currentSize = GetFileSize(filePath);
            ofs.open(filePath, std::ios::out | std::ios::app);
        }
    }

    inline void Append(const std::string &str)
    {
        if (ofs.is_open()) {
            ofs << str;
            ofs.flush();
            currentSize += str.length();
        }
    }

    inline void LogStr(const std::string &str)
    {
        if (outType == LogOutType::BOTH || outType == LogOutType::TERMINAL) {
            (*outMap[level]) << str;
        }
        if (outType == LogOutType::BOTH || outType == LogOutType::FILE) {
            Check();
            Append(str);
        }
    }

    template <typename T> inline void LogT(const T &t)
    {
        std::string str = GetString(t);
        LogStr(str);
    }

    int maxSize = 10 * 1024 * 1024;
    const int maxCount = 10;
    volatile int currentSize = 0;
    volatile int count = 0;
    volatile bool isAppend = true;
    std::ofstream ofs;
    std::string originFilePath;
    std::string filePath;
    LogOutType outType = LogOutType::BOTH;
    LogLevel level = LogLevel::L_INFO;
    std::string wsPort;
    std::map<LogLevel, std::ostream *> outMap = { { LogLevel::L_INFO, &std::cout },
                                                  { LogLevel::L_WARN, &std::cout },
                                                  { LogLevel::L_DEBUG, &std::cout },
                                                  { LogLevel::L_ERROR, &std::cerr },
                                                  { LogLevel::L_FATAL, &std::cerr } };
};
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_LOG_H
