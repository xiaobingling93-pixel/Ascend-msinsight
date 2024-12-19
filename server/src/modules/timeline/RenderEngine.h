//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_RENDERENGINE_H
#define PROFILER_SERVER_RENDERENGINE_H
#include <unordered_set>
#include "RenderEngineInterface.h"
namespace Dic::Module::Timeline {
class RenderEngine : public RenderEngineInterface {
public:
    static std::shared_ptr<RenderEngine> Instance()
    {
        static std::shared_ptr<RenderEngine> instance = std::make_shared<RenderEngine>();
        return instance;
    }
    RenderEngine() = default;
    RenderEngine(const RenderEngine &) = delete;
    RenderEngine &operator = (const RenderEngine &) = delete;
    RenderEngine(RenderEngine &&) = delete;
    RenderEngine &operator = (RenderEngine &&) = delete;
    ~RenderEngine() override = default;
    void SetDataEngineInterface(std::shared_ptr<DataEngineInterface>) override;
    void QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
        Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId) override;
    bool QueryFlowCategoryEvents(Protocol::FlowCategoryEventsParams &params, uint64_t minTimestamp,
        std::vector<std::unique_ptr<Protocol::UnitSingleFlow>> &flowDetailList) override;
    void QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
        Protocol::UnitThreadDetailBody &responseBody, uint64_t trackId) override;
    CompeteSliceDomain FindSliceByTimePoint(const std::string &fileId, const std::string &name, uint64_t timePoint,
        const std::string &metaType) override;

private:
    std::shared_ptr<DataEngineInterface> dataEngine = nullptr;
    const std::unordered_set<std::string> hideAbleNameSet = { "SET_FLAG", "WAIT_FLAG", "set_event", "wait_event" };

    void ComputeSimulationFlows(const Protocol::FlowCategoryEventsParams &params,
        std::vector<std::unique_ptr<Protocol::UnitSingleFlow>> &flowDetailList,
        std::vector<FlowPoint> &flowPointResult);
};
}


#endif // PROFILER_SERVER_RENDERENGINE_H
