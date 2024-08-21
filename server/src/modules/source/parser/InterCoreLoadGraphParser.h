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
    uint64_t throughput = 0;
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
    std::map<std::string, uint64_t> minCycleMap;
    std::map<std::string, uint64_t> minThroughputMap;
    std::map<std::string, float> maxHitRateMap;

    void AddOpDetail(InterCoreOpDetail&& opDetail)
    {
        opDetails.emplace_back(std::move(opDetail));
    }

    void SetMinCycle(const std::string &subCoreType, uint64_t cycle)
    {
        if (cycle <= 0) {
            return;
        }
        uint64_t &minCycle = minCycleMap[subCoreType];
        if (minCycle == 0) {
            minCycle = cycle;
            return;
        }
        minCycle = std::min(minCycle, cycle);
    }

    void SetMinThroughput(const std::string &subCoreType, uint64_t throughput)
    {
        if (throughput <= 0) {
            return;
        }
        uint64_t &minThroughput = minThroughputMap[subCoreType];
        if (minThroughput == 0) {
            minThroughput = throughput;
            return;
        }
        minThroughput = std::min(minThroughput, throughput);
    }

    void SetMaxHitRate(const std::string &subCoreType, float hitRate)
    {
        float &maxHitRate = maxHitRateMap[subCoreType];
        maxHitRate = (NumberUtil::IsGreater(hitRate, maxHitRate) ? hitRate : maxHitRate);
    }
};

class InterCoreLoadGraphParser {
public:
    bool GetInterCoreLoadAnalysisInfo(const std::string& json, Dic::Protocol::DetailsInterCoreLoadGraphBody& body);

private:
    std::optional<InterCoreLoadAnalysisDetail> ParseInterCoreLoadAnalysisInfo(const std::string& json);
    void ParseJsonOpDetailArray(InterCoreLoadAnalysisDetail &analysisDetail, const json_t &jsonOpDetailArray);
    void TransformAnalysisDetail(InterCoreLoadAnalysisDetail &analysisDetail);
    void Try2MoveSubCoreDetails(InterCoreOpDetail &source, InterCoreOpDetail &dest, uint8_t subCoreIndex);
    const uint8_t SUB_CORE_INDEX_0 = 0;
    const uint8_t SUB_CORE_INDEX_1 = 1;
};

} // Dic
} // Module
} // Source

#endif // PROFILER_SERVER_INTERCORELOADGRAPHPARSER_H
