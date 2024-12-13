/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEMORY_COMPONENT_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_COMPONENT_HANDLER_H

#include "MemoryRequestHandler.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "VirtualMemoryDataBase.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryMemoryComponentHandler : public MemoryRequestHandler {
public:
    QueryMemoryComponentHandler()
    {
        command = Protocol::REQ_RES_MEMORY_COMPONENT;
    }
    ~QueryMemoryComponentHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
    void GetComponentDiff(const std::vector<MemoryComponent> &compareData,
        const std::vector<MemoryComponent> &baselineData, std::vector<MemoryComponentComparison> &diffData);
    void SelectResult(MemoryComponentRequest &request,
        MemoryComponentComparisonResponse &response,
        std::vector<MemoryComponentComparison> &fullDiffResult);
    void SortResult(MemoryComponentRequest &request, std::vector<MemoryComponentComparison> &result);
private:
    bool CompareComponent(std::shared_ptr<VirtualMemoryDataBase> database,
        std::shared_ptr<VirtualMemoryDataBase> databaseBaseline,
        MemoryComponentRequest &request, MemoryComponentComparisonResponse &response, std::string &errorMsg);
    void Merge(MemoryComponent &componentCompare, MemoryComponent &componentBaseline,
        MemoryComponentComparison &mergeResult);
    void SortAscend(MemoryComponentRequest &request, std::vector<MemoryComponentComparison> &result);
    void SortDescend(MemoryComponentRequest &request, std::vector<MemoryComponentComparison> &result);
    const std::vector<Protocol::MemoryTableColumnAttr> tableColumnAttr = {
        {"Component", "string", "component"},
        {"Peak Memory Reserved(MB)", "number", "totalReserved"},
        {"Timestamp(ms)", "number", "timestamp"}
    };
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_MEMORY_COMPONENT_HANDLER_H
