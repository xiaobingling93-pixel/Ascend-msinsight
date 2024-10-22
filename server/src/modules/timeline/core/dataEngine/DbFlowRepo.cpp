/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "TrackInfoManager.h"
#include "DbFlowRepo.h"
namespace Dic::Module::Timeline {
void DbFlowRepo::QueryFlowPointByCategory(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    if (flowQuery.cat == "async_task_queue") {
        hostFlowRepo->QueryAsyncTaskQueue(flowQuery, flowPointVec);
    } else if (flowQuery.cat == "fwdbwd") {
        hostFlowRepo->QueryFwdbwd(flowQuery, flowPointVec);
    } else if (flowQuery.cat == "async_npu") {
        QueryAsyncNpu(flowQuery, flowPointVec);
    } else if (flowQuery.cat == "HostToDevice") {
        QueryHostToDevice(flowQuery, flowPointVec);
    } else if (flowQuery.cat == "MsTx") {
        QueryMsTx(flowQuery, flowPointVec);
    }
}

void DbFlowRepo::QueryFlowPointByTimeRange(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) {}

void DbFlowRepo::QueryFlowPointByFlowId(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) {}

void DbFlowRepo::QueryHostToDevice(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    hostFlowRepo->AddCannFlowPoint(flowQuery, flowPointVec);
    deviceFlowRepo->AddDeviceFlowPoint(flowQuery, flowPointVec);
    std::sort(flowPointVec.begin(), flowPointVec.end());
}

void DbFlowRepo::QueryMsTx(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    std::vector<uint64_t> connectionIds = hostFlowRepo->AddMstxFlowPoint(flowQuery, flowPointVec);
    deviceFlowRepo->AddHardWareMstxFlowPoint(flowQuery, flowPointVec, connectionIds);
    std::sort(flowPointVec.begin(), flowPointVec.end());
}

void DbFlowRepo::QueryAsyncNpu(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    hostFlowRepo->AddAsyncNpuFlowPoint(flowQuery, flowPointVec);
    deviceFlowRepo->AddDeviceFlowPoint(flowQuery, flowPointVec);
    std::sort(flowPointVec.begin(), flowPointVec.end());
}
}
