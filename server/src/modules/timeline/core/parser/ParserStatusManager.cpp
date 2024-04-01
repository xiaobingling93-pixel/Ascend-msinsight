/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ParserStatusManager.h"
#include "ConstantDefs.h"
#include "algorithm"

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

ParserStatus ParserStatusManager::GetClusterParserStatus()
{
    std::unique_lock<std::mutex> lock(mutex);
    return clusterParseStatus;
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
    if (statusMap[fileId] != ParserStatus::RUNNING && statusMap[fileId] != ParserStatus::TERMINATE) {
        return false;
    }
    statusMap[fileId] = ParserStatus::FINISH;
    parseCv.notify_all();
    return true;
}

ParserStatus ParserStatusManager::SetTerminateStatus(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (statusMap.count(fileId) == 0) {
        return ParserStatus::UN_KNOW;
    }
    auto oldStatus = statusMap[fileId];
    if (oldStatus != ParserStatus::FINISH) {
        statusMap[fileId] = ParserStatus::TERMINATE;
    }
    return oldStatus;
}

void ParserStatusManager::SetAllTerminateStatus()
{
    for (auto &statu : statusMap) {
        if (statu.second != ParserStatus::FINISH) {
            statu.second = ParserStatus::TERMINATE;
        }
    }
}

void ParserStatusManager::SetClusterParseStatus(ParserStatus parserStatus)
{
    std::unique_lock<std::mutex> lock(mutex);
    clusterParseStatus = parserStatus;
}

bool ParserStatusManager::IsAllFinished()
{
    std::unique_lock<std::mutex> lock(mutex);
    for (const auto &item : statusMap) {
        if (item.second != ParserStatus::FINISH) {
            return false;
        }
    }
    if (clusterParseStatus != ParserStatus::FINISH) {
        return false;
    }
    return true;
}

bool ParserStatusManager::IsFinished(const std::string &fileId)
{
    return statusMap.count(fileId) == 0 || statusMap[fileId] == ParserStatus::FINISH;
}

void ParserStatusManager::WaitAllFinished(const std::vector<std::string> &fileIds)
{
    std::unique_lock<std::mutex> lock(mutex);
    auto func = [this](const std::string &fileId) {
        return !IsFinished(fileId) || !IsFinished(KERNEL_PREFIX + fileId) ||
               !IsFinished(MEMORY_PREFIX + fileId);
    };
    while (std::any_of(fileIds.begin(), fileIds.end(), func)) {
        parseCv.wait_for(lock, std::chrono::seconds(2)); // 最长等待时间2秒
    }
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic