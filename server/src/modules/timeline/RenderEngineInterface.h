//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_RENDERENGINEINTERFACE_H
#define PROFILER_SERVER_RENDERENGINEINTERFACE_H
#include <memory>
#include "TimelineProtocolRequest.h"
#include "TimelineProtocolResponse.h"
#include "DataEngineInterface.h"
namespace Dic {
namespace Module {
namespace Timeline {
class RenderEngineInterface {
public:
    virtual ~RenderEngineInterface() = default;
    virtual void SetDataEngineInterface(std::shared_ptr<DataEngineInterface>) = 0;
    virtual bool QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
        Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId) = 0;
};
}
}
}
#endif // PROFILER_SERVER_RENDERENGINEINTERFACE_H
