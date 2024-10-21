/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SERVER_LOG_H
#define DATA_INSIGHT_CORE_SERVER_LOG_H

#include <memory>
#include "LogUtil.h"


namespace Dic {
namespace Server {
using namespace Dic;
class ServerLog {
public:
    static inline void Initialize(const std::string &logPath, const int &logSize, const std::string &logLevelStr,
                                  std::string wsPortStr)
    {
        ServerLog &serverLog = Instance();
        serverLog.wsPort = wsPortStr;
        if (serverLog.printInstance == nullptr) {
            serverLog.printInstance = std::make_unique<LogUtil>(LogOutType::TERMINAL, "", wsPortStr);
        }
        if (serverLog.recordInstance == nullptr) {
            std::string logFileName = logPath + "/profiler_server.log";
            serverLog.recordInstance = std::make_unique<LogUtil>(LogOutType::FILE, logFileName, wsPortStr, logSize);
            LogLevel logLevel = LogUtil::GetLogLevel(logLevelStr);
            serverLog.recordInstance->SetLogLevel(logLevel);
        }
    }

    template <typename... ARGS> static inline void Info(const ARGS... args)
    {
        Instance().Record(LogLevel::L_INFO, args...);
#ifndef NDEBUG
        Instance().Print(LogLevel::L_INFO, args...);
#endif
    }

    template <typename... ARGS> static inline void Debug(const ARGS... args)
    {
        Instance().Record(LogLevel::L_DEBUG, args...);
#ifndef NDEBUG
        Instance().Print(LogLevel::L_DEBUG, args...);
#endif
    }

    template <typename... ARGS> static inline void Warn(const ARGS... args)
    {
        Instance().Record(LogLevel::L_WARN, args...);
#ifndef NDEBUG
        Instance().Print(LogLevel::L_WARN, args...);
#endif
    }

    template <typename... ARGS> static inline void Error(const ARGS... args)
    {
        Instance().Record(LogLevel::L_ERROR, args...);
#ifndef NDEBUG
        Instance().Print(LogLevel::L_ERROR, args...);
#endif
    }

    template <typename... ARGS> static inline void PrintInfo(const ARGS... args)
    {
        Instance().Print(LogLevel::L_INFO, args...);
    }

    template <typename... ARGS> static inline void PrintWarn(const ARGS... args)
    {
        Instance().Print(LogLevel::L_WARN, args...);
    }

    template <typename... ARGS> static inline void PrintError(const ARGS... args)
    {
        Instance().Print(LogLevel::L_ERROR, args...);
    }

private:
    ServerLog() = default;
    ~ServerLog() = default;

    static ServerLog &Instance()
    {
        static ServerLog instance;
        return instance;
    }

    static const std::string GetLogHead(const LogLevel &level)
    {
        std::string head;
        head.append(LogPrefix::Instance().TimePrefix());
        head.append("|");
        head.append(LogPrefix::Instance().LevelPrefix(level));
        head.append(" ");
        return head;
    }

    template <typename... ARGS> inline void Record(const LogLevel &level, const ARGS... args)
    {
        std::lock_guard<std::mutex> lock(recordInstanceMutex);
        if (recordInstance == nullptr) {
            std::string logPath = "./profiler_server.log";
            const int logSize = 32 * 1024 * 1024;
            recordInstance = std::make_unique<LogUtil>(LogOutType::FILE, logPath, wsPort);
            recordInstance->SetLogLevel(level).SetMaxSize(logSize);
        }
        std::string head = GetLogHead(level);
        recordInstance->LogT(level, head, args...);
        recordInstance->SetWsPort(wsPort);
    }

    template <typename... ARGS> inline void Print(const LogLevel &level, const ARGS... args)
    {
        std::lock_guard<std::mutex> lock(printInstanceMutex);
        if (printInstance == nullptr) {
            printInstance = std::make_unique<LogUtil>(LogOutType::TERMINAL, "", wsPort);
        }
        std::string head = GetLogHead(level);
        printInstance->LogT(level, head, args...);
    }

    std::unique_ptr<LogUtil> printInstance = nullptr;
    std::unique_ptr<LogUtil> recordInstance = nullptr;
    std::mutex recordInstanceMutex;
    std::mutex printInstanceMutex;
    std::string wsPort;
};
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SERVER_LOG_H
