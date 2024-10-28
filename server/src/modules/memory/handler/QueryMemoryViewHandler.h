/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEMORY_COMPONENT_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_COMPONENT_HANDLER_H

#include "WsSession.h"
#include "MemoryRequestHandler.h"
#include "MemoryProtocolRespose.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryMemoryViewHandler : public MemoryRequestHandler {
public:
    QueryMemoryViewHandler()
    {
        command = Protocol::REQ_RES_MEMORY_VIEW;
    };
    ~QueryMemoryViewHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    bool GetCompareGraph(VirtualMemoryDataBase *database, VirtualMemoryDataBase *databaseBaseline,
        MemoryViewRequest &request, std::unique_ptr<MemoryViewResponse> &responsePtr, Server::WsSession &session);
    void GetCompareGraphLines(const Protocol::MemoryViewData &compareData,
                              const Protocol::MemoryViewData &baselineData,
                              Protocol::MemoryViewData &resultData);
    void GetCompareGraphLegends(const Protocol::MemoryViewData &compareData,
                                const Protocol::MemoryViewData &baselineData,
                                Protocol::MemoryViewData &resultData);
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_QUERY_MEMORY_COMPONENT_HANDLER_H
