/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "TrackInfoManager.h"
#include "DeviceFlowRepo.h"
namespace Dic::Module::Timeline {
void DeviceFlowRepo::AddDeviceFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    std::unordered_map<uint64_t, uint64_t> opIdMap = QueryOpIdMap(flowQuery);
    std::unordered_map<uint64_t, uint64_t> deviceMap = QueryDeviceMap(flowQuery);
    std::string host = hostInfoTable->GetHost(flowQuery.fileId);
    std::unordered_set<uint64_t> hcclConnectionIdSet =
        AddGroupHcclFlowPoint(flowQuery, flowPointVec, opIdMap, deviceMap, host);
    AddHardWareFlowPoint(flowQuery, flowPointVec, host, hcclConnectionIdSet);
}

void DeviceFlowRepo::AddHardWareMstxFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec,
    const std::vector<uint64_t> &connectionIds)
{
    std::vector<TaskPO> taskPOS;
    taskTable->Select(TaskColumn::ROW_ID, TaskColumn::TIMESTAMP)
        .Select(TaskColumn::CONNECTION_ID, TaskColumn::STREAM_ID, TaskColumn::DECICED_ID)
        .In(TaskColumn::CONNECTION_ID, connectionIds)
        .ExcuteQuery(flowQuery.fileId, taskPOS);
    std::string host = hostInfoTable->GetHost(flowQuery.fileId);
    auto &instance = TrackInfoManager::Instance();
    for (const auto &item : taskPOS) {
        uint64_t realConnectionId = item.connectionId;
        std::string flowId = std::to_string(realConnectionId);
        FlowPoint endPoint;
        endPoint.type = "f";
        endPoint.flowId = flowId;
        endPoint.id = item.id;
        endPoint.timestamp = item.timestamp - flowQuery.minTimestamp;
        endPoint.rankId = host + instance.GetRankId(host, std::to_string(item.deviceId));
        endPoint.trackId = instance.GetTrackId(endPoint.rankId, hardWarePid, std::to_string(item.streamId));
        flowPointVec.emplace_back(endPoint);
    }
}

std::unordered_set<uint64_t> DeviceFlowRepo::AddGroupHcclFlowPoint(const FlowQuery &flowQuery,
    std::vector<FlowPoint> &flowPointVec, const std::unordered_map<uint64_t, uint64_t> &opIdMap,
    const std::unordered_map<uint64_t, uint64_t> &deviceMap, const std::string &host)
{
    std::vector<CommucationTaskOpPO> commucationTaskOpPOs;
    commucationOpTable->Select(CommucationTaskOpColumn::OP_ID, CommucationTaskOpColumn::TIMESTAMP)
        .Select(CommucationTaskOpColumn::CONNECTION_ID, CommucationTaskOpColumn::GROUPNAME)
        .ExcuteQuery(flowQuery.fileId, commucationTaskOpPOs);
    std::unordered_set<uint64_t> hcclConnectionIdSet;
    auto &instance = TrackInfoManager::Instance();
    for (const auto &item : commucationTaskOpPOs) {
        FlowPoint endPoint;
        if (opIdMap.count(item.opId) == 0 || deviceMap.count(opIdMap.at(item.opId)) == 0) {
            continue;
        }
        endPoint.rankId = host + instance.GetRankId(host, std::to_string(deviceMap.at(opIdMap.at(item.opId))));
        uint64_t realConnectionId = item.connectionId;
        std::string flowId = std::to_string(realConnectionId);
        hcclConnectionIdSet.emplace(realConnectionId);
        endPoint.type = "f";
        endPoint.flowId = flowId;
        endPoint.id = item.opId;
        endPoint.timestamp = item.timestamp - flowQuery.minTimestamp;
        endPoint.trackId = instance.GetTrackId(endPoint.rankId, hcclPid, std::to_string(item.groupName) + "group");
        flowPointVec.emplace_back(endPoint);
    }
    return hcclConnectionIdSet;
}

std::unordered_map<uint64_t, uint64_t> DeviceFlowRepo::QueryDeviceMap(const FlowQuery &flowQuery)
{
    std::vector<TaskPO> deviceTaskPOS;
    taskTable->Select(TaskColumn::GLOBAL_TASK_ID, TaskColumn::DECICED_ID).ExcuteQuery(flowQuery.fileId, deviceTaskPOS);
    std::unordered_map<uint64_t, uint64_t> deviceMap;
    for (const auto &item : deviceTaskPOS) {
        deviceMap[item.globalTaskId] = item.deviceId;
    }
    return deviceMap;
}

std::unordered_map<uint64_t, uint64_t> DeviceFlowRepo::QueryOpIdMap(const FlowQuery &flowQuery)
{
    std::vector<CommucationTaskInfoPO> commucationTaskInfoPOs;
    commucationTaskInfoTable->Select(CommucationTaskInfoColumn::GLOBAL_TASK_ID)
        .Select(CommucationTaskInfoColumn::OP_ID)
        .GroupBy(CommucationTaskInfoColumn::OP_ID)
        .ExcuteQuery(flowQuery.fileId, commucationTaskInfoPOs);
    std::unordered_map<uint64_t, uint64_t> opIdMap;
    for (const auto &item : commucationTaskInfoPOs) {
        opIdMap[item.opId] = item.globalTaskId;
    }
    return opIdMap;
}

void DeviceFlowRepo::AddHardWareFlowPoint(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec,
    const std::string &host, const std::unordered_set<uint64_t> &hcclConnectionIdSet)
{
    std::vector<TaskPO> taskPOS;
    taskTable->Select(TaskColumn::ROW_ID, TaskColumn::TIMESTAMP)
        .Select(TaskColumn::CONNECTION_ID, TaskColumn::STREAM_ID, TaskColumn::DECICED_ID)
        .ExcuteQuery(flowQuery.fileId, taskPOS);
    auto &instance = TrackInfoManager::Instance();
    for (const auto &item : taskPOS) {
        uint64_t realConnectionId = item.connectionId;
        std::string flowId = std::to_string(realConnectionId);
        if (hcclConnectionIdSet.count(realConnectionId) > 0) {
            continue;
        }
        FlowPoint endPoint;
        endPoint.type = "f";
        endPoint.flowId = flowId;
        endPoint.id = item.id;
        endPoint.timestamp = item.timestamp - flowQuery.minTimestamp;
        endPoint.rankId = host + instance.GetRankId(host, std::to_string(item.deviceId));
        endPoint.trackId = instance.GetTrackId(endPoint.rankId, hardWarePid, std::to_string(item.streamId));
        flowPointVec.emplace_back(endPoint);
    }
}
}
