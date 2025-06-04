/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_TOPNADVICEMAINTAINER_H
#define PROFILER_SERVER_TOPNADVICEMAINTAINER_H

#include <functional>
#include <queue>
#include "ClusterDef.h"

namespace Dic::Module::Summary {

double GetTotalSynchronizeTime(const AdviceInfoForSlowRank &info);

/**
 * 用一个小顶堆，维护总同步时间为TopN的元素
 */
class TopNAdviceMaintainer {
public:
    explicit TopNAdviceMaintainer(uint32_t len) : length(len),
        cmp([](const AdviceInfoForSlowRank &a, const AdviceInfoForSlowRank &b) {
            return GetTotalSynchronizeTime(a) > GetTotalSynchronizeTime(b); // 同步时间越小，越优先出队
        }), queue(cmp) {}

    void Insert(const AdviceInfoForSlowRank &info);
    std::vector<AdviceInfoForSlowRank> GetTopNSlowest(uint32_t topN) const;
    bool IsEmpty() const;

private:
    uint32_t length;
    std::function<bool(const AdviceInfoForSlowRank&, const AdviceInfoForSlowRank&)> cmp;
    std::priority_queue<AdviceInfoForSlowRank, std::vector<AdviceInfoForSlowRank>, decltype(cmp)> queue;
};
}

#endif // PROFILER_SERVER_TOPNADVICEMAINTAINER_H
