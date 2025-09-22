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
        async = true;
        command = Protocol::REQ_RES_MEMORY_VIEW;
    };
    ~QueryMemoryViewHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
    bool GetRespectiveData(std::shared_ptr<VirtualMemoryDataBase> database,
                           MemoryViewData &compareData, MemoryViewData &baselineData,
                           MemoryViewRequest &request, std::string &errorMsg);
    void ExecuteComparisonAlgorithm(const MemoryViewData &compareData, const MemoryViewData &baselineData,
                                    MemoryViewResponse &response);
    void GetCompareGraphLines(const Protocol::MemoryViewData &compareData,
                              const Protocol::MemoryViewData &baselineData,
                              Protocol::MemoryViewData &resultData);
    void GetCompareGraphLegends(const Protocol::MemoryViewData &compareData,
                                const Protocol::MemoryViewData &baselineData,
                                Protocol::MemoryViewData &resultData);

    bool QueryCurveData(MemoryViewResponse &response, std::string &errorMsg, MemoryViewRequest &request,
                        std::unique_ptr<MemoryViewResponse> &responsePtr);

    void AddBaseLineData(const MemoryViewData &baselineData, MemoryViewData &resultData, size_t indexBaseline) const;
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_QUERY_MEMORY_VIEW_HANDLER_H
