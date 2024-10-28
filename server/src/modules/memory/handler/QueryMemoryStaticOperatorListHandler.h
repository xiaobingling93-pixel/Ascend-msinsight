/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEMORY_STATIC_OPERATOR_LIST_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_STATIC_OPERATOR_LIST_HANDLER_H

#include "MemoryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryMemoryStaticOperatorListHandler : public MemoryRequestHandler {
public:
    QueryMemoryStaticOperatorListHandler()
    {
        command = Protocol::REQ_RES_MEMORY_STATIC_OP_MEMORY_LIST;
    };
    ~QueryMemoryStaticOperatorListHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    bool CompareOperator(VirtualMemoryDataBase *database, VirtualMemoryDataBase *databaseBaseline,
        MemoryStaticOperatorListRequest &request, std::unique_ptr<MemoryStaticOperatorListCompResponse> &responsePtr,
        Server::WsSession &session);
    void GetOperatorDiff(const MemoryStaticOperatorListResponse &compareData,
                         const MemoryStaticOperatorListResponse &baselineData,
                         Protocol::MemoryStaticOperatorListCompResponse &resultData);
    void VectorMerge(std::vector<StaticOperatorItem> &compareVec, std::vector<StaticOperatorItem> &baselineVec,
                     MemoryStaticOperatorListCompResponse &diffData);
    void Subtract(StaticOperatorCompItem &element);
    bool SelectDiffResult(MemoryStaticOperatorListRequest &request,
                          std::unique_ptr<MemoryStaticOperatorListCompResponse> &responsePtr,
                          MemoryStaticOperatorListCompResponse &fullDiffResult, Server::WsSession &session);
    bool IsSelected(MemoryStaticOperatorListRequest &request, const StaticOperatorCompItem &op);
    void SortResult(MemoryStaticOperatorListRequest &request, MemoryStaticOperatorListCompResponse &result);
    void SortAscend(MemoryStaticOperatorListRequest &request, MemoryStaticOperatorListCompResponse &result);
    void SortDescend(MemoryStaticOperatorListRequest &request, MemoryStaticOperatorListCompResponse &result);
    const std::vector<Protocol::MemoryTableColumnAttr> staticOpTableColumnAttr = {
        {"Device ID", "string", "deviceId"},
        {"Name", "string", "opName"},
        {"Node Index Start", "number", "nodeIndexStart"},
        {"Node Index End", "number", "nodeIndexEnd"},
        {"Size(MB)", "number", "size"}
    };
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_MEMORY_STATIC_OPERATOR_LIST_HANDLER_H
