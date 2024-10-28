/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEMORY_OPERATOR_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_OPERATOR_HANDLER_H

#include "MemoryRequestHandler.h"
#include "VirtualMemoryDataBase.h"
#include "WsSession.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryMemoryOperatorHandler : public MemoryRequestHandler {
public:
    QueryMemoryOperatorHandler()
    {
        command = Protocol::REQ_RES_MEMORY_OPERATOR;
    };
    ~QueryMemoryOperatorHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    bool CompareOperator(VirtualMemoryDataBase *database, VirtualMemoryDataBase *databaseBaseline,
        MemoryOperatorRequest &request, std::unique_ptr<MemoryOperatorComparisonResponse> &responsePtr,
        Server::WsSession &session);
    void GetOperatorDiff(const MemoryOperatorResponse &compareData,
                         const MemoryOperatorResponse &baselineData,
                         Protocol::MemoryOperatorComparisonResponse &resultData);
    void VectorMerge(std::vector<MemoryOperator> &compareVec, std::vector<MemoryOperator> &baselineVec,
                     MemoryOperatorComparisonResponse &diffData);
    void Subtract(MemoryOperatorComparison &element);
    bool SelectDiffResult(MemoryOperatorRequest &request,
                          std::unique_ptr<MemoryOperatorComparisonResponse> &responsePtr,
                          MemoryOperatorComparisonResponse &fullDiffResult, Server::WsSession &session);
    bool IsSelected(MemoryOperatorRequest &request, const MemoryOperatorComparison &op);
    void SortResult(MemoryOperatorRequest &request, MemoryOperatorComparisonResponse &result);
    void SortAscend(MemoryOperatorRequest &request, MemoryOperatorComparisonResponse &result);
    void SortDescend(MemoryOperatorRequest &request, MemoryOperatorComparisonResponse &result);
    const std::vector<Protocol::MemoryTableColumnAttr> tableColumnAttr = {
        {"Name", "string", "name"},
        {"Size(KB)", "number", "size"},
        {"Allocation Time(ms)", "number", "allocationTime"},
        {"Release Time(ms)", "number", "releaseTime"},
        {"Duration(ms)", "number", "duration"},
        {"Active Release Time(ms)", "number", "activeReleaseTime"},
        {"Active Duration(ms)", "number", "activeDuration"},
        {"Allocation Total Allocated(MB)", "number", "allocationAllocated"},
        {"Allocation Total Reserved(MB)", "number", "allocationReserved"},
        {"Allocation Total Active(MB)", "number", "allocationActive"},
        {"Release Total Allocated(MB)", "number", "releaseAllocated"},
        {"Release Total Reserved(MB)", "number", "releaseReserved"},
        {"Release Total Active(MB)", "number", "releaseActive"},
        {"Stream", "string", "streamId"}
    };
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_MEMORY_OPERATOR_HANDLER_H
