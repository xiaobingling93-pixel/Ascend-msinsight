/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SummaryService.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "NumberUtil.h"

namespace Dic {
namespace Module {
namespace Summary {
bool SummaryService::QuerySummaryBaseInfo(SummaryBaseInfo &baseInfo, std::shared_ptr<VirtualClusterDatabase> &db)
{
    // db在外层进行校验 必不为空
    if (!db->QueryBaseInfo(baseInfo)) {
        ServerLog::Warn("Fail to query summary base info.");
        return false;
    }
    uint64_t min = UINT64_MAX;
    uint64_t max = 0;
    db->QueryExtremumTimestamp(min, max);
    if (min > max) {
        ServerLog::Warn("Fail to get extremum timestamp when query summary base info.");
        return false;
    }
    baseInfo.collectStartTime =
        NumberUtil::CeilingClamp(min / (numberThousands * numberThousands), (uint64_t)INT64_MAX);
    baseInfo.collectDuration = NumberUtil::CeilingClamp((max - min) / numberThousands, (uint64_t)INT64_MAX);
    return true;
}
void SummaryService::QueryCompareSummaryBaseInfo(const SummaryTopRankRequest &request, SummaryTopRankResponse &response)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr || !QuerySummaryBaseInfo(response.body.baseInfo.compare, database)) {
        ServerLog::Warn("Fail to query compare summary base info");
    }

    if (!request.params.isCompare) {
        return;
    }

    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
    if (baselineDatabase == nullptr || !QuerySummaryBaseInfo(response.body.baseInfo.baseline, baselineDatabase)) {
        ServerLog::Warn("Fail to query baseline summary base info");
    }
}
}
}
}
