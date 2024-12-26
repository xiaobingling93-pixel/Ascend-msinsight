/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARYSERVICE_H
#define PROFILER_SERVER_SUMMARYSERVICE_H
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Protocol;

class SummaryService {
public:
    static void QueryCompareSummaryBaseInfo(const SummaryTopRankRequest &request, SummaryTopRankResponse &response);
    static bool QuerySummaryBaseInfo(SummaryBaseInfo &baseInfo, std::shared_ptr<VirtualClusterDatabase> &db);
private:
    static inline int numberThousands = 1000;
};
}
}
}

#endif // PROFILER_SERVER_SUMMARYSERVICE_H
