/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SYSTEM_UTIL_H
#define PROFILER_SERVER_SYSTEM_UTIL_H

#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace Dic {
class SystemUtil {
public:
    static unsigned int GetCpuCoreCount()
    {
#ifdef _WIN32
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return sysInfo.dwNumberOfProcessors;
#else
        return sysconf(_SC_NPROCESSORS_CONF);
#endif
    }

    static std::string GetTempDir()
    {
        std::string tempDir;

        const char *tempEnvVars[] = {"TMPDIR", "TEMP", "TMP"};
        const int numVars = sizeof(tempEnvVars) / sizeof(tempEnvVars[0]);

        for (int i = 0; i < numVars; ++i) {
            const char *envValue = std::getenv(tempEnvVars[i]);
            if (envValue) {
                tempDir = envValue;
                break;
            }
        }

        if (tempDir.empty()) {
#ifdef _WIN32
            tempDir = "C:\\Temp";
#else
            tempDir = "/tmp";
#endif
        }

        return tempDir;
    }
};
}
#endif // PROFILER_SERVER_SYSTEM_UTIL_H