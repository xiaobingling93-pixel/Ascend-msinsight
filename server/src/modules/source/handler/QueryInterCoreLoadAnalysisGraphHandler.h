/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYINTERCORELOADANALYSISGRAPHHANDLER_H
#define PROFILER_SERVER_QUERYINTERCORELOADANALYSISGRAPHHANDLER_H
#include "SourceRequestHandler.h"

namespace Dic {
namespace Module {
namespace Source {

class QueryInterCoreLoadAnalysisGraphHandler : public SourceRequestHandler {
public:
    QueryInterCoreLoadAnalysisGraphHandler()
    {
        command = Protocol::REQ_RES_DETAILS_INTER_CORE_LOAD_GRAPH;
    }
    ~QueryInterCoreLoadAnalysisGraphHandler() override = default;

    bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override;
};

} // Dic
} // Module
} // Source

#endif // PROFILER_SERVER_QUERYINTERCORELOADANALYSISGRAPHHANDLER_H
