/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ParserStatusManager.h"

namespace Dic {
namespace Module {
namespace Timeline {

ParserStatusManager &ParserStatusManager::Instance()
{
    static ParserStatusManager instance;
    return instance;
}

void ParserStatusManager::SetParserStatus(const std::string &fileId, ParserStatus status)
{
    std::unique_lock<std::mutex> lock(mutex);
    statusMap[fileId] = status;
}

void ParserStatusManager::ClearParserStatus(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    statusMap.erase(fileId);
}

void ParserStatusManager::ClearAllParserStatus()
{
    std::unique_lock<std::mutex> lock(mutex);
    statusMap.clear();
}

ParserStatus ParserStatusManager::GetParserStatus(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (statusMap.count(fileId) == 0) {
        return ParserStatus::UN_KNOW;
    }
    return statusMap[fileId];
}

bool ParserStatusManager::SetRunningStatus(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (statusMap.count(fileId) == 0) {
        return false;
    }
    if (statusMap[fileId] != ParserStatus::INIT) {
        return false;
    }
    statusMap[fileId] = ParserStatus::RUNNING;
    return true;
}

bool ParserStatusManager::SetFinishStatus(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (statusMap.count(fileId) == 0) {
        return false;
    }
    if (statusMap[fileId] != ParserStatus::RUNNING) {
        return false;
    }
    statusMap[fileId] = ParserStatus::FINISH;
    return true;
}

ParserStatus ParserStatusManager::SetTerminateStatus(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (statusMap.count(fileId) == 0) {
        return ParserStatus::UN_KNOW;
    }
    auto oldStatus = statusMap[fileId];
    statusMap[fileId] = ParserStatus::TERMINATE;
    return oldStatus;
}

} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic