//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_HOSTFLOWREPO_H
#define PROFILER_SERVER_HOSTFLOWREPO_H
#include "PytorchApiTable.h"
#include "ConnectionIdsTable.h"
#include "StringIdsTable.h"
#include "MstxEventsTable.h"
#include "CannApiTable.h"
namespace Dic::Module::Timeline {
class HostFlowRepo {
public:
    void QueryFwdbwd(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec);
    void QueryAsyncTaskQueue(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec);
    void AddAsyncNpuFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec);
    void AddCannFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec);
    std::vector<uint64_t> AddMstxFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec);

private:
    const std::string pythonApiTid = "pytorch";
    const std::string mstxTid = "MsTx";
    std::unique_ptr<PytorchApiTable> pytorchApiTable = std::make_unique<PytorchApiTable>();
    std::unique_ptr<ConnectionIdsTable> connectionIdsTable = std::make_unique<ConnectionIdsTable>();
    std::unique_ptr<StringIdsTable> stringIdsTable = std::make_unique<StringIdsTable>();
    std::unique_ptr<MstxEventsTable> mstxEventsTable = std::make_unique<MstxEventsTable>();
    std::unique_ptr<CannApiTable> cannApiTable = std::make_unique<CannApiTable>();
    uint64_t QueryEnqueueNameId(const FlowQuery &flowQuery);
    std::vector<PytorchApiPO> QueryNotEnqueuePythonApi(const FlowQuery &flowQuery, uint64_t nameId);
    std::unordered_map<uint64_t, uint64_t> QueryConnectionIdMap(const FlowQuery &flowQuery);
    std::vector<uint64_t> QueryEnqueueFlowConnectionIds(const FlowQuery &flowQuery, uint64_t nameId);
    std::vector<uint64_t> QueryRealConnectionIds(const FlowQuery &flowQuery,
        const std::vector<uint64_t> &pythonConnectionIds);
    void QueryAllPythonConnectionIds(const FlowQuery &flowQuery, const std::vector<uint64_t> &realConnectionIds,
        std::unordered_map<uint64_t, uint64_t> &connectionIdMap, std::vector<uint64_t> &allPythonConnectionIds);
};
}
#endif // PROFILER_SERVER_HOSTFLOWREPO_H
