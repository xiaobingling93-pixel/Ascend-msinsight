/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACE_TIME_H
#define PROFILER_SERVER_TRACE_TIME_H

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>
namespace Dic {
namespace Module {
namespace Timeline {
struct CardGroup {
    uint64_t groupMinTime = 0;
    uint64_t groupMaxTime = 0;
    bool operator < (const CardGroup &right) const
    {
        if (groupMinTime < right.groupMinTime) {
            return true;
        }
        return groupMinTime == right.groupMinTime && groupMaxTime < right.groupMaxTime;
    }
};
class TraceTime {
public:
    static TraceTime &Instance();
    void Reset();
    void UpdateTime(uint64_t min, uint64_t max);
    void UpdateCardTimeDuration(const std::string &fileId, uint64_t min, uint64_t max);
    std::vector<std::pair<std::string, uint64_t>> ComputeCardMinTimeInfo();
    uint64_t GetOffsetByFileId(const std::string &fileId);
    uint64_t GetStartTime();
    uint64_t GetDuration();
    void SetIsSimulation(bool simulation);
    bool GetIsSimulation();

private:
    TraceTime();
    ~TraceTime() = default;
    std::mutex mutex;
    uint64_t maxTimestamp{};
    uint64_t minTimestamp{};
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> cardTimeDurationMap;
    std::vector<CardGroup> cardGroupTimeDurations;
    bool isSimulation = false;
    void UpdateCardGroupTime(uint64_t min, uint64_t max);

    uint64_t ComputetargetOffset(const std::pair<uint64_t, uint64_t> &targetTime);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_TRACE_TIME_H
