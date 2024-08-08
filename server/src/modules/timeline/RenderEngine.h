//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_RENDERENGINE_H
#define PROFILER_SERVER_RENDERENGINE_H
#include "SliceAnalyzer.h"
#include "RenderEngineInterface.h"
namespace Dic {
namespace Module {
namespace Timeline {
class RenderEngine : public RenderEngineInterface {
public:
    static std::shared_ptr<RenderEngine> Instance()
    {
        static std::shared_ptr<RenderEngine> instance = std::make_shared<RenderEngine>();
        return instance;
    }
    RenderEngine()
    {
        if (sliceAnalyzerPtr == nullptr) {
            sliceAnalyzerPtr = std::make_unique<SliceAnalyzer>();
        }
    }
    RenderEngine(const RenderEngine &) = delete;
    RenderEngine &operator = (const RenderEngine &) = delete;
    RenderEngine(RenderEngine &&) = delete;
    RenderEngine &operator = (RenderEngine &&) = delete;
    ~RenderEngine() override = default;
    void SetDataEngineInterface(std::shared_ptr<DataEngineInterface>) override;
    bool QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
        Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId) override;

private:
    std::shared_ptr<DataEngineInterface> dataEngine = nullptr;
    std::unique_ptr<SliceAnalyzer> sliceAnalyzerPtr = nullptr;
};
}
}
}
#endif // PROFILER_SERVER_RENDERENGINE_H
