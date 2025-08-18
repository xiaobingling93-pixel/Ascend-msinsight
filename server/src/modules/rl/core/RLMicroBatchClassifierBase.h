/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLMICROBATCHCLASSIFIERBASE_H
#define PROFILER_SERVER_RLMICROBATCHCLASSIFIERBASE_H
#include <vector>
#include "RLProtocolResponse.h"
#include "RLDomainObject.h"

namespace Dic::Module::RL {
enum State : int {
    Init = 0,
    FP = 1,
    BP = 2,
    Communication = 3
};
/**
 * @brief 算法基类，用于支持不同后端的不同microBatch划分逻辑
 */
class RLMicroBatchClassifierBase {
public:
    /**
     * @brief 入口函数
     */
    std::vector<Protocol::RLPipelineNode> GetClassifiedMicroBatch(const std::string &fileId, const RLMstxConfig &config,
            const Protocol::RLPipelineNode &taskNode)
    {
        auto original = QueryMicroBatchSlices(fileId, config, taskNode);
        return MicroBatchClassifier(original);
    }

protected:

    virtual std::vector<Protocol::RLPipelineNode> QueryMicroBatchSlices(const std::string &fileId,
            const RLMstxConfig &config,
            const Protocol::RLPipelineNode &taskNode) = 0;

    virtual std::vector<Protocol::RLPipelineNode> MicroBatchClassifier(
            std::vector<Protocol::RLPipelineNode> &nodes) = 0;
};
}
#endif // PROFILER_SERVER_RLMICROBATCHCLASSIFIERBASE_H
