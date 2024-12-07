/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEMORY_OPERATOR_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_OPERATOR_HANDLER_H

#include "MemoryRequestHandler.h"
#include "VirtualMemoryDataBase.h"

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
    bool GetRespectiveData(std::shared_ptr<VirtualMemoryDataBase> database,
        std::vector<MemoryOperator> &compareData, std::vector<MemoryOperator> &baselineData,
        MemoryOperatorRequest &request, std::unique_ptr<MemoryOperatorComparisonResponse> &responsePtr);
    void ExecuteComparisonAlgorithm(std::vector<MemoryOperator> &compareData, std::vector<MemoryOperator> &baselineData,
                                    MemoryOperatorRequest &request,
                                    std::unique_ptr<MemoryOperatorComparisonResponse> &responsePtr);
    void GetOperatorDiff(const std::vector<MemoryOperator> &compareData,
                         const std::vector<MemoryOperator> &baselineData,
                         std::vector<MemoryOperatorComparison> &resultData);
    void SelectDiffResult(MemoryOperatorRequest &request,
                          std::unique_ptr<MemoryOperatorComparisonResponse> &responsePtr,
                          std::vector<MemoryOperatorComparison> &fullDiffResult);
    void SortResult(MemoryOperatorRequest &request, MemoryOperatorComparisonResponse &result);
private:
    void VectorMerge(std::vector<MemoryOperator> &compareVec, std::vector<MemoryOperator> &baselineVec,
                     std::vector<MemoryOperatorComparison> &diffData);
    void Subtract(MemoryOperatorComparison &element);
    bool IsSelected(MemoryOperatorRequest &request, const MemoryOperatorComparison &op);
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
