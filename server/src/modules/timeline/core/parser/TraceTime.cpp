/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {
TraceTime &TraceTime::Instance()
{
    static TraceTime instance;
    return instance;
}

void TraceTime::Reset()
{
    std::unique_lock<std::mutex> lock(mutex);
    maxTimestamp = 0;
    minTimestamp = UINT64_MAX;
    cardMinTimeMap.clear();
    isSimulation = false;
}

TraceTime::TraceTime()
{
    Reset();
}

void TraceTime::UpdateTime(uint64_t min, uint64_t max)
{
    std::unique_lock<std::mutex> lock(mutex);
    minTimestamp = std::min(minTimestamp, min);
    maxTimestamp = std::max(maxTimestamp, max);
}

uint64_t TraceTime::GetStartTime()
{
    std::unique_lock<std::mutex> lock(mutex);
    return minTimestamp;
}

uint64_t TraceTime::GetDuration()
{
    std::unique_lock<std::mutex> lock(mutex);
    uint64_t maxOffset = 0;
    for (const auto &item: cardMinTimeMap) {
        maxOffset = std::max(maxOffset, item.second - minTimestamp);
    }
    if (minTimestamp < UINT64_MAX - maxOffset && maxTimestamp >= minTimestamp + maxOffset) {
        return maxTimestamp - minTimestamp - maxOffset;
    } else {
        Server::ServerLog::Warn("Max timestamp is less than min timestamp. Max timestamp:", maxTimestamp,
            ", min timestamp:", minTimestamp);
        return 0;
    }
}

void TraceTime::UpdateCardMinTime(const std::string &fileId, uint64_t min)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (cardMinTimeMap.count(fileId) == 0) {
        cardMinTimeMap[fileId] = min;
    } else {
        cardMinTimeMap[fileId] = std::min(cardMinTimeMap[fileId], min);
    }
    minTimestamp = std::min(minTimestamp, cardMinTimeMap[fileId]);
}

std::vector<std::pair<std::string, uint64_t>> TraceTime::ComputeCardMinTimeInfo()
{
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<std::pair<std::string, uint64_t>> result;
    for (const auto &item : cardMinTimeMap) {
        uint64_t offset = item.second - minTimestamp;
        result.emplace_back(item.first, offset);
    }
    return result;
}

uint64_t TraceTime::GetOffsetByFileId(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    uint64_t cardMinTime = cardMinTimeMap[fileId];
    if (cardMinTime > minTimestamp) {
        return (cardMinTime - minTimestamp);
    }
    return 0;
}

void TraceTime::SetIsSimulation(bool simulation)
{
    std::unique_lock<std::mutex> lock(mutex);
    isSimulation = simulation;
}

bool TraceTime::GetIsSimulation()
{
    std::unique_lock<std::mutex> lock(mutex);
    return isSimulation;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic