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
};
}
#endif // PROFILER_SERVER_SYSTEM_UTIL_H