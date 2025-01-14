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
    bool GetRespectiveData(std::shared_ptr<VirtualMemoryDataBase> database,
        std::vector<StaticOperatorItem> &compareData, std::vector<StaticOperatorItem> &baselineData,
        MemoryStaticOperatorListRequest &request, std::string &errorMsg);
    void ExecuteComparisonAlgorithm(const std::vector<StaticOperatorItem> &compareData,
        const std::vector<StaticOperatorItem> &baselineData, MemoryStaticOperatorListRequest &request,
        MemoryStaticOperatorListCompResponse &response);
    void GetOperatorDiff(const std::vector<StaticOperatorItem> &compareData,
                         const std::vector<StaticOperatorItem> &baselineData,
                         std::vector<StaticOperatorCompItem> &resultData);
    void SelectDiffResult(MemoryStaticOperatorListRequest &request,
                          MemoryStaticOperatorListCompResponse &response,
                          std::vector<StaticOperatorCompItem> &fullDiffResult);
    void SortResult(MemoryStaticOperatorListRequest &request, MemoryStaticOperatorListCompResponse &result);
private:
    void VectorMerge(std::vector<StaticOperatorItem> &compareVec, std::vector<StaticOperatorItem> &baselineVec,
                     std::vector<StaticOperatorCompItem> &diffData);
    void Subtract(StaticOperatorCompItem &element);
    bool IsSelected(MemoryStaticOperatorListRequest &request, const StaticOperatorCompItem &op);
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
