/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACEIDMANAGER_H
#define PROFILER_SERVER_TRACEIDMANAGER_H
#include <string>
namespace Dic {
class TraceIdManager {
public:
    static std::string GetTraceId();
    static void SetTraceId(const std::string &traceId);
    static std::string GenerateTraceId();

private:
    static thread_local std::string threadLocalTraceId;
};
}
#endif // PROFILER_SERVER_TRACEIDMANAGER_H
