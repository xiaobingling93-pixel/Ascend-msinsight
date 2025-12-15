/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_FINDSLICEBYALLOCATIONTIMEHANDLER_H
#define PROFILER_SERVER_FINDSLICEBYALLOCATIONTIMEHANDLER_H
#include "RenderEngineInterface.h"
#include "OperatorMemoryService.h"
#include "MemoryRequestHandler.h"

namespace Dic::Module::Memory {
class FindSliceByAllocationTimeHandler : public MemoryRequestHandler {
public:
    explicit FindSliceByAllocationTimeHandler(
        std::shared_ptr<Timeline::RenderEngineInterface> renderPtr,
        std::shared_ptr<OperatorMemoryService> operatorMemoryServicePtr = std::make_shared<OperatorMemoryService>())
        : renderEngine(std::move(renderPtr)),
          operatorMemoryService(std::move(operatorMemoryServicePtr))
    {
        command = Protocol::REQ_RES_MEMORY_FIND_SLICE;
    };

    ~FindSliceByAllocationTimeHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

private:
    std::shared_ptr<Timeline::RenderEngineInterface> renderEngine = nullptr;
    std::shared_ptr<OperatorMemoryService> operatorMemoryService = nullptr;

    void populateResponseData(MemoryFindSliceResponse& response, OperatorDomain& target,
                              Timeline::CompeteSliceDomain& slice);
};
}  // namespace Dic::Module::Memory

#endif  // PROFILER_SERVER_FINDSLICEBYALLOCATIONTIMEHANDLER_H
