/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#ifndef PROFILER_SERVER_QUERY_MEMORY_STATIC_OPERATOR_GRAPH_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_STATIC_OPERATOR_GRAPH_HANDLER_H

#include "WsSession.h"
#include "MemoryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryMemoryStaticOperatorGraphHandler : public MemoryRequestHandler {
public:
    QueryMemoryStaticOperatorGraphHandler()
    {
        command = Protocol::REQ_RES_MEMORY_STATIC_OP_MEMORY_GRAPH;
    };
    ~QueryMemoryStaticOperatorGraphHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    bool GetCompareGraph(std::shared_ptr<VirtualMemoryDataBase> database,
        std::shared_ptr<VirtualMemoryDataBase> databaseBaseline, MemoryStaticOperatorGraphRequest &request,
        MemoryStaticOperatorGraphResponse &response, std::string &errorMsg);
    void GetCompareGraphLines(const Protocol::StaticOperatorGraphItem &compareData,
                         const Protocol::StaticOperatorGraphItem &baselineData,
                         Protocol::StaticOperatorGraphItem &resultData);
    void GetCompareGraphLegends(const Protocol::StaticOperatorGraphItem &compareData,
                                const Protocol::StaticOperatorGraphItem &baselineData,
                                Protocol::StaticOperatorGraphItem &resultData);
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_MEMORY_STATIC_OPERATOR_GRAPH_HANDLER_H
