/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "TraceIdManager.h"
#include <random>

namespace Dic {
thread_local std::string TraceIdManager::threadLocalTraceId;

std::string TraceIdManager::GetTraceId()
{
    return threadLocalTraceId;
}

void TraceIdManager::SetTraceId(const std::string &traceId)
{
    threadLocalTraceId = traceId;
}

std::string TraceIdManager::GenerateTraceId()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    return std::to_string(dis(gen));
}
}