/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEMORY_VIEW_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_VIEW_HANDLER_H

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
    void GetCompareGraphLines(const Protocol::MemoryViewData &compareData,
                              const Protocol::MemoryViewData &baselineData,
                              Protocol::MemoryViewData &resultData);
    void GetCompareGraphLegends(const Protocol::MemoryViewData &compareData,
                                const Protocol::MemoryViewData &baselineData,
                                Protocol::MemoryViewData &resultData);
private:
    bool GetCompareGraph(std::shared_ptr<VirtualMemoryDataBase> database,
                         std::shared_ptr<VirtualMemoryDataBase> databaseBaseline, MemoryViewRequest &request,
                         MemoryViewResponse &response, std::string &errorMsg);
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_QUERY_MEMORY_VIEW_HANDLER_H
