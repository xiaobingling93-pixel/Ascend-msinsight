/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_COLLECTIONTIMESERVICE_H
#define PROFILER_SERVER_COLLECTIONTIMESERVICE_H
#include <string>
#include <mutex>
#include <vector>
namespace Dic::Module::Timeline {
struct HostTimeStruct {
    std::string hostWithOutMark;
    std::string hostPath;
    uint64_t startTime = UINT64_MAX;
    uint64_t endTime = 0;
    uint32_t mark = 0;
};
class CollectionTimeService {
public:
    static CollectionTimeService &Instance()
    {
        static CollectionTimeService instance;
        return instance;
    }
    CollectionTimeService(const CollectionTimeService &) = delete;
    CollectionTimeService &operator = (const CollectionTimeService &) = delete;
    CollectionTimeService(CollectionTimeService &&) = delete;
    CollectionTimeService &operator = (CollectionTimeService &&) = delete;
    std::string ComputeMarkHost(std::string &hostWithOutMark, const std::string &hostPath,
        uint64_t startTime, uint64_t endTime)
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::string result = hostWithOutMark;
        bool isAddHostTime = true;
        uint32_t mark = 0;
        for (auto &item : hostTimeVec) {
            if (item.hostWithOutMark != hostWithOutMark) {
                continue;
            }
            mark++;
            bool isSamePath = item.hostPath == hostPath;
            bool isOverLap = !((startTime > item.endTime) || (endTime < item.startTime));
            if (isSamePath && isOverLap) {
                isAddHostTime = false;
                mark = item.mark;
                item.startTime = std::min(item.startTime, startTime);
                item.endTime = std::max(item.endTime, endTime);
                break;
            }
        }
        if (isAddHostTime) {
            HostTimeStruct temp = { hostWithOutMark, hostPath, startTime, endTime, mark };
            hostTimeVec.emplace_back(temp);
        }
        result = result + "_" + std::to_string(mark) + " ";
        return result;
    }

    void Reset()
    {
        std::unique_lock<std::mutex> lock(mutex);
        hostTimeVec.clear();
    }

private:
    CollectionTimeService() = default;
    ~CollectionTimeService() = default;
    std::vector<HostTimeStruct> hostTimeVec;
    std::mutex mutex;
};
}
#endif // PROFILER_SERVER_COLLECTIONTIMESERVICE_H
