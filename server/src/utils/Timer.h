/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */
#include <string>
#include <chrono>
#include "ServerLog.h"
// 性能测试工具类，利用生命周期进行测试
namespace Dic {
class Timer {
public:
    Timer(const std::string &name) : m_Name(name), m_Stopped(false)
    {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    void Stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();
        long long start =
            std::chrono::time_point_cast<std::chrono::milliseconds>(m_StartTimepoint).time_since_epoch().count();
        long long end =
            std::chrono::time_point_cast<std::chrono::milliseconds>(endTimepoint).time_since_epoch().count();
        Server::ServerLog::Info("Performance : ", m_Name, ": ", end - start, "ms");
        m_Stopped = true;
    }

    ~Timer()
    {
        if (!m_Stopped) {
            Stop();
        }
    }

private:
    std::string m_Name;
    bool m_Stopped;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
};
}