/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLPIPELINESERVICE_H
#define PROFILER_SERVER_RLPIPELINESERVICE_H

#include <unordered_map>
#include <vector>
#include <set>
#include "pch.h"
#include "RLProtocolResponse.h"
#include "VirtualTraceDatabase.h"
#include "DomainObject.h"
#include "RLDomainObject.h"
#include "RLMicroBatchMegatronClassifier.h"

namespace Dic::Module::RL {
class RLPipelineService {
public:
    static RLPipelineService& Instance();
    bool GetPipelineInfo(Protocol::RLPipelineResponse &response);
private:
    void Clear();
    std::vector<Protocol::RLPipelineNode> SearchNode(const std::string &rankId);
    std::vector<Protocol::RLPipelineNode> QueryMicroBatchByTask(const std::string &fileId,
            const std::vector<Protocol::RLPipelineNode> &taskList, const RLMstxConfig &taskConfig);
    void QueryPipelineByRankId(const std::string &rankIdWithHost);
    void FillPipelineMap(const std::string &originHostName, const std::string &rankId,
                                const std::vector<Protocol::RLPipelineNode> &pipeline,
                                std::unordered_map<std::string, RLPipelineItem> &targetMap);
    void FillAndProcessPipelineData(std::unordered_map<std::string, RLPipelineItem> &pipelineMap,
                                           std::vector<RLPipelineItem> &pipelineData);
    std::vector<Protocol::RLPipelineNode> QueryMicroBatch(const std::string &fileId, const RLMstxConfig &config,
            const Protocol::RLPipelineNode &node);

    RLBackEndType GetBackendType(const std::string& rankId);
    std::mutex mtx;
    uint64_t minTime;
    uint64_t maxTime;
    std::set<std::string> stageTypeList;
    std::unordered_map<std::string, RLPipelineItem> taskPipelineMap;
    std::unordered_map<std::string, RLPipelineItem> microBatchPipelineMap;
    RLBackEndType rlBackEndType{RLBackEndType::Unknown};
};
}
#endif // PROFILER_SERVER_RLPIPELINESERVICE_H
