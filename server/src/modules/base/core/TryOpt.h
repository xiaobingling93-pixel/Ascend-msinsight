/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRYOPT_H
#define PROFILER_SERVER_TRYOPT_H
#include "ServerLog.h"
namespace Dic {
namespace Module {
template <typename T>
bool TryOpt(const std::shared_ptr<T>& ptr, const std::string& msg)
{
    if (!ptr) {
        Dic::Server::ServerLog::Warn(msg);
        return false;
    }
    return true;
}

template <typename T>
bool TryOpt(const std::unique_ptr<T>& ptr, const std::string& msg)
{
    if (!ptr) {
        Dic::Server::ServerLog::Warn(msg);
        return false;
    }
    return true;
}

}  // namespace Module
}  // namespace Dic
#endif  // PROFILER_SERVER_TRYOPT_H
