// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "pch.h"
#include "TableDefs.h"
#include "TrackInfoManager.h"
#include "HcclRepo.h"
namespace Dic::Module::Timeline {
void HcclRepo::QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery, std::vector<SliceDomain> &sliceVec)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        ServerLog::Warn("hccl query all slice track info is not exist, track is: ", sliceQuery.trackId);
        return;
    }
    std::string sql;
    const std::string suffix = "group";
    if (StringUtil::EndWith(trackInfo.threadId, suffix)) {
        QuerySimpleSliceFromGroupTrack(sliceVec, trackInfo, suffix);
    } else {
        QuerySimpleSliceFromPlaneTrack(sliceVec, trackInfo);
    }
}

void HcclRepo::QuerySimpleSliceFromPlaneTrack(std::vector<SliceDomain> &sliceVec, TrackInfo &trackInfo)
{
    std::vector<CommucationTaskInfoPO> commucationTaskInfoPoVec;
    std::string groupName = trackInfo.threadId;
    std::string threadId = trackInfo.threadId;
    size_t pos = trackInfo.threadId.find_last_of("_");
    if (pos != std::string::npos && trackInfo.threadId.size() > pos) {
        threadId = trackInfo.threadId.substr(pos + 1);
        groupName = trackInfo.threadId.substr(0, pos);
    }
    commucationTaskInfoTable->Select(CommucationTaskInfoColumn::GLOBAL_TASK_ID)
        .Eq(CommucationTaskInfoColumn::GROUPNAME, groupName)
        .Eq(CommucationTaskInfoColumn::PLANE_ID, threadId)
        .ExcuteQuery(trackInfo.cardId, commucationTaskInfoPoVec);
    std::vector<uint64_t> globalTaskIds(commucationTaskInfoPoVec.size());
    std::transform(commucationTaskInfoPoVec.begin(), commucationTaskInfoPoVec.end(), globalTaskIds.begin(),
        [](const CommucationTaskInfoPO &item) { return item.globalTaskId; });
    std::vector<TaskPO> taskPoVec;
    taskTable->Select(TaskColumn::ROW_ID, TaskColumn::TIMESTAMP)
        .Select(TaskColumn::ENDTIME)
        .Eq(TaskColumn::DECICED_ID, trackInfo.rankId)
        .In(TaskColumn::GLOBAL_TASK_ID, globalTaskIds)
        .ExcuteQuery(trackInfo.cardId, taskPoVec);
    for (const auto &item : taskPoVec) {
        SliceDomain sliceDomain;
        sliceDomain.id = item.id;
        sliceDomain.timestamp = item.timestamp;
        sliceDomain.endTime = item.endTime;
        sliceVec.emplace_back(sliceDomain);
    }
}

void HcclRepo::QuerySimpleSliceFromGroupTrack(std::vector<SliceDomain> &sliceVec, const TrackInfo &trackInfo,
    const std::string &suffix)
{
    std::vector<uint64_t> globalIds = QueryGlobalTaskIdsByRank(trackInfo);
    std::vector<uint64_t> opIds = QueryOpIdsByGlabalTaskIds(trackInfo, globalIds);
    std::string tid = trackInfo.threadId.substr(0, trackInfo.threadId.size() - suffix.size());
    std::vector<CommucationTaskOpPO> commucationTaskOpPOVec;
    commucationOpTable->Select(CommucationTaskOpColumn::TIMESTAMP, CommucationTaskOpColumn::ENDTIME)
        .Select(CommucationTaskOpColumn::OP_ID)
        .Eq(CommucationTaskOpColumn::GROUPNAME, tid)
        .In(CommucationTaskOpColumn::OP_ID, opIds)
        .ExcuteQuery(trackInfo.cardId, commucationTaskOpPOVec);
    for (const auto &item : commucationTaskOpPOVec) {
        SliceDomain sliceDomain;
        sliceDomain.id = item.opId;
        sliceDomain.timestamp = item.timestamp;
        sliceDomain.endTime = item.endTime;
        sliceVec.emplace_back(sliceDomain);
    }
}

std::vector<uint64_t> HcclRepo::QueryOpIdsByGlabalTaskIds(const TrackInfo &trackInfo,
    const std::vector<uint64_t> &globalIds)
{
    std::vector<CommucationTaskInfoPO> commucationTaskInfoPoVec;
    commucationTaskInfoTable->Select(CommucationTaskInfoColumn::OP_ID)
        .In(CommucationTaskInfoColumn::GLOBAL_TASK_ID, globalIds)
        .GroupBy(CommucationTaskInfoColumn::OP_ID)
        .ExcuteQuery(trackInfo.cardId, commucationTaskInfoPoVec);
    std::vector<uint64_t> opIds(commucationTaskInfoPoVec.size());
    std::transform(commucationTaskInfoPoVec.begin(), commucationTaskInfoPoVec.end(), opIds.begin(),
        [](const CommucationTaskInfoPO &item) { return item.opId; });
    return opIds;
}

std::vector<uint64_t> HcclRepo::QueryGlobalTaskIdsByRank(const TrackInfo &trackInfo)
{
    std::vector<TaskPO> taskPoVec;
    taskTable->Select(TaskColumn::GLOBAL_TASK_ID)
        .Eq(TaskColumn::DECICED_ID, trackInfo.rankId)
        .ExcuteQuery(trackInfo.cardId, taskPoVec);
    std::vector<uint64_t> globalIds(taskPoVec.size());
    std::transform(taskPoVec.begin(), taskPoVec.end(), globalIds.begin(),
        [](const TaskPO &item) { return item.globalTaskId; });
    return globalIds;
}

void HcclRepo::QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds) {}
uint64_t HcclRepo::QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery)
{
    return 0;
}
void HcclRepo::QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
    std::vector<CompeteSliceDomain> &sliceVec)
{}

void HcclRepo::QueryAllThreadInfo(const ThreadQuery &flowQuery,
    std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo)
{}

void HcclRepo::QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
    std::vector<CompeteSliceDomain> &competeSliceVec)
{
    if (std::empty(sliceIds)) {
        return;
    }
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        return;
    }
    const std::string suffix = "group";
    if (StringUtil::EndWith(trackInfo.threadId, suffix)) {
        QueryGroupSliceByIds(sliceIds, competeSliceVec, trackInfo);
    } else {
        QueryPlaneSliceByIds(sliceIds, competeSliceVec, trackInfo);
    }
}

void HcclRepo::QueryPlaneSliceByIds(const std::vector<uint64_t> &sliceIds,
    std::vector<CompeteSliceDomain> &competeSliceVec, const TrackInfo &trackInfo)
{
    std::vector<TaskPO> taskPoVec;
    taskTable->Select(TaskColumn::ROW_ID, TaskColumn::TIMESTAMP)
        .Select(TaskColumn::ENDTIME, TaskColumn::GLOBAL_TASK_ID)
        .In(TaskColumn::ROW_ID, sliceIds)
        .ExcuteQuery(trackInfo.cardId, taskPoVec);
    std::string nameKey = taskTable->GetDbPath(trackInfo.cardId);
    std::vector<uint64_t> globalIds(taskPoVec.size());
    std::transform(taskPoVec.begin(), taskPoVec.end(), globalIds.begin(),
        [](const TaskPO &item) { return item.globalTaskId; });
    std::vector<CommucationTaskInfoPO> commucationTaskInfoPoVec;
    commucationTaskInfoTable->Select(CommucationTaskInfoColumn::GLOBAL_TASK_ID)
        .Select(CommucationTaskInfoColumn::TASK_TYPE)
        .In(CommucationTaskInfoColumn::GLOBAL_TASK_ID, globalIds)
        .ExcuteQuery(trackInfo.cardId, commucationTaskInfoPoVec);
    std::unordered_map<uint64_t, uint64_t> typeNameMap;
    for (const auto &item : commucationTaskInfoPoVec) {
        typeNameMap[item.globalTaskId] = item.taskType;
    }
    for (const auto &item : taskPoVec) {
        CompeteSliceDomain competeSliceDomain;
        competeSliceDomain.id = item.id;
        competeSliceDomain.timestamp = item.timestamp;
        competeSliceDomain.endTime = item.endTime;
        competeSliceDomain.name =
            FullDb::DbTraceDataBase::GetStringCacheValue(nameKey, std::to_string(typeNameMap[item.globalTaskId]));
        competeSliceVec.emplace_back(competeSliceDomain);
    }
}

void HcclRepo::QueryGroupSliceByIds(const std::vector<uint64_t> &sliceIds,
    std::vector<CompeteSliceDomain> &competeSliceVec, const TrackInfo &trackInfo)
{
    std::vector<CommucationTaskOpPO> commucationTaskOpPOVec;
    commucationOpTable->Select(CommucationTaskOpColumn::OP_ID, CommucationTaskOpColumn::TIMESTAMP)
        .Select(CommucationTaskOpColumn::ENDTIME, CommucationTaskOpColumn::OP_NAME)
        .In(CommucationTaskOpColumn::OP_ID, sliceIds)
        .ExcuteQuery(trackInfo.cardId, commucationTaskOpPOVec);
    std::string nameKey = commucationOpTable->GetDbPath(trackInfo.cardId);
    for (const auto &item : commucationTaskOpPOVec) {
        CompeteSliceDomain competeSliceDomain;
        competeSliceDomain.id = item.opId;
        competeSliceDomain.timestamp = item.timestamp;
        competeSliceDomain.endTime = item.endTime;
        competeSliceDomain.name = FullDb::DbTraceDataBase::GetStringCacheValue(nameKey, std::to_string(item.opName));
        competeSliceVec.emplace_back(competeSliceDomain);
    }
}

void HcclRepo::SetTaskTable(std::unique_ptr<TaskTable> taskTablePtr)
{
    if (taskTablePtr != nullptr) {
        taskTable = std::move(taskTablePtr);
    }
}

void HcclRepo::SetCommucationOpTable(std::unique_ptr<CommucationOpTable> commucationOpTablePtr)
{
    if (commucationOpTablePtr != nullptr) {
        commucationOpTable = std::move(commucationOpTablePtr);
    }
}

void HcclRepo::SetCommucationTaskInfoTable(std::unique_ptr<CommucationTaskInfoTable> commucationTaskInfoTablePtr)
{
    if (commucationTaskInfoTablePtr != nullptr) {
        commucationTaskInfoTable = std::move(commucationTaskInfoTablePtr);
    }
}
}
