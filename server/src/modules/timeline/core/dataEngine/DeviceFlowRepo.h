/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DEVICEFLOWREPO_H
#define PROFILER_SERVER_DEVICEFLOWREPO_H
#include "CommucationOpTable.h"
#include "TaskTable.h"
#include "HostInfoTable.h"
#include "NpuInfoRepo.h"
#include "CommucationTaskInfoTable.h"
namespace Dic::Module::Timeline {
class DeviceFlowRepo {
public:
    void AddDeviceFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec);
    void AddHardWareMstxFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec,
        const std::vector<uint64_t> &connectionIds);
    void SetTaskTable(std::unique_ptr<TaskTable> taskTablePtr);
    void SetCommucationOpTable(std::unique_ptr<CommucationOpTable> commucationOpTablePtr);
    void SetNpuInfoRepo(std::unique_ptr<NpuInfoRepo> npuInfoRepoPtr);

private:
    const std::string hcclPid = "HCCL";
    const std::string hardWarePid = "Ascend Hardware";
    std::unique_ptr<CommucationOpTable> commucationOpTable = std::make_unique<CommucationOpTable>();
    std::unique_ptr<TaskTable> taskTable = std::make_unique<TaskTable>();
    std::unique_ptr<HostInfoTable> hostInfoTable = std::make_unique<HostInfoTable>();
    std::unique_ptr<CommucationTaskInfoTable> commucationTaskInfoTable = std::make_unique<CommucationTaskInfoTable>();
    std::unique_ptr<NpuInfoRepo> npuInfoRepo = std::make_unique<NpuInfoRepo>();
    std::unordered_map<uint64_t, uint64_t> QueryOpIdMap(const FlowQuery &flowQuery);
    std::unordered_map<uint64_t, uint64_t> QueryDeviceMap(const FlowQuery &flowQuery);
    std::unordered_set<uint64_t> AddGroupHcclFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec,
        const std::unordered_map<uint64_t, uint64_t> &opIdMap, const std::unordered_map<uint64_t, uint64_t> &deviceMap,
        const std::string &host);
    void AddHardWareFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec, const std::string &host,
        const std::unordered_set<uint64_t> &hcclConnectionIdSet);
};
}
#endif // PROFILER_SERVER_DEVICEFLOWREPO_H
