/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_INTERCORELOADGRAPHPARSER_H
#define PROFILER_SERVER_INTERCORELOADGRAPHPARSER_H

#include <string>
#include <vector>
#include "SourceProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Source {

struct InterCoreSubCoreDetail {
    std::string subCoreType;
    uint8_t subCoreIndex = 0;
    uint64_t cycles = 0;
    float throughput = 0;
    float hitRate = 0;
};

struct InterCoreOpDetail {
    uint8_t coreId = 0;
    std::vector<InterCoreSubCoreDetail> subCoreDetails = {};
};

struct InterCoreLoadAnalysisDetail {
    std::string soc;
    std::string opType;
    std::string advice;
    std::vector<InterCoreOpDetail> opDetails = {};
    uint64_t minCycle = 0;
    float minThroughput = 0;
    float maxHitRate = 0;

    void AddOpDetail(InterCoreOpDetail&& opDetail)
    {
        opDetails.emplace_back(std::move(opDetail));
    }

    void SetMinCycle(uint64_t cycle)
    {
        if (cycle <= 0) {
            return;
        }
        if (minCycle == 0) {
            minCycle = cycle;
            return;
        }
        minCycle = std::min(minCycle, cycle);
    }

    void SetMinThroughput(float throughput)
    {
        if (throughput <= 0) {
            return;
        }
        if (minThroughput == 0) {
            minThroughput = throughput;
            return;
        }
        minThroughput = (NumberUtil::IsGreater(minThroughput, throughput) ? throughput : minThroughput);
    }

    void SetMaxHitRate(float hitRate)
    {
        maxHitRate = (NumberUtil::IsGreater(hitRate, maxHitRate) ? hitRate : maxHitRate);
    }
};

class InterCoreLoadGraphParser {
public:
    bool GetInterCoreLoadAnalysisInfo(const std::string& json, Dic::Protocol::DetailsInterCoreLoadGraphBody& body);

private:
    InterCoreLoadAnalysisDetail ParseInterCoreLoadAnalysisInfo(const std::string& json);
};

} // Dic
} // Module
} // Source

#endif // PROFILER_SERVER_INTERCORELOADGRAPHPARSER_H
