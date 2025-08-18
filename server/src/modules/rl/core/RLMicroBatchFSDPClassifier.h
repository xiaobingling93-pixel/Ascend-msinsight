/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef PROFILER_SERVER_RLMICROBATCHFSDPCLASSIFIER_H
#define PROFILER_SERVER_RLMICROBATCHFSDPCLASSIFIER_H
#include "RLMicroBatchClassifierBase.h"
#include "PythonApiRepo.h"
namespace Dic::Module::RL {
using namespace Dic::Module::Timeline;
class RLMicroBatchFSDPClassifier : public RLMicroBatchClassifierBase {
public:
    virtual ~RLMicroBatchFSDPClassifier() = default;
protected:
    std::vector<Protocol::RLPipelineNode> QueryMicroBatchSlices(const std::string &fileId, const RLMstxConfig &config,
                                                                const Protocol::RLPipelineNode &taskNode) override;

    std::vector<Protocol::RLPipelineNode> MicroBatchClassifier(std::vector<Protocol::RLPipelineNode> &nodes) override;
private:

    std::vector<RLPipelineNode> QueryFPSlices(const std::string &rankId, const RLPipelineNode &taskNode);

    std::vector<RLPipelineNode> QueryBPSlices(const std::string &rankId, const RLPipelineNode &taskNode);

    std::vector<RLPipelineNode> TransSliceToNodes(
        const std::vector<CompeteSliceDomain> &slices, const RLPipelineNode &task, const std::string &name,
        const std::string &nodeType);

    std::vector<RLPipelineNode> NodeSortMerge(const std::vector<RLPipelineNode> &left,
                                              const std::vector<RLPipelineNode> &right);

    bool IsHappenBefore(std::vector<RLPipelineNode>::const_iterator left,
                        std::vector<RLPipelineNode>::const_iterator right);

    void ProcessInitState(const RLPipelineNode &node, std::vector<RLPipelineNode> &res);

    void ProcessFPState(const RLPipelineNode &node, std::vector<RLPipelineNode> &res);

    void ProcessBPState(const RLPipelineNode &node, std::vector<RLPipelineNode> &res);

    void ProcessCommunicationState(const RLPipelineNode &node, std::vector<RLPipelineNode> &res);

    RLPipelineNode current;
    State state{State::Init};
    inline static std::string FP_MICRO_BATCH_NAME = "FullyShardedDataParallel.forward";
    inline static std::string BP_MICRO_BATCH_NAME = "autograd::engine";
    inline static std::string BP_MICRO_BATCH_END_NAME = "Enqueue@record_event";
};
}

#endif // PROFILER_SERVER_RLMICROBATCHFSDPCLASSIFIER_H
