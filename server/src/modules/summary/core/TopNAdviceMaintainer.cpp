/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "TopNAdviceMaintainer.h"

namespace Dic::Module::Summary {

double GetTotalSynchronizeTime(const AdviceInfoForSlowRank &info)
{
    double total = 0.0;
    for (const auto &[pg, t] : info.synchronizeTime) {
        (void)(pg);
        total += t;
    }
    return total;
}

void TopNAdviceMaintainer::Insert(const AdviceInfoForSlowRank &info)
{
    queue.push(info);
    if (queue.size() > length) queue.pop();
}

std::vector<AdviceInfoForSlowRank> TopNAdviceMaintainer::GetTopNSlowest(uint32_t topN) const
{
    // 拷贝堆内容
    std::vector<AdviceInfoForSlowRank> all;
    auto tempQueue = queue;
    while (!tempQueue.empty()) {
        all.push_back(tempQueue.top());
        tempQueue.pop();
    }
    std::reverse(all.begin(), all.end());

    // 截取前 topN
    if (static_cast<size_t>(topN) < all.size()) {
        all.resize(topN);
    }
    return all;
}

bool TopNAdviceMaintainer::IsEmpty() const
{
    return queue.empty();
}
}