/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_HCCLREPO_MOCK_DATA_H
#define PROFILER_SERVER_HCCLREPO_MOCK_DATA_H
#include <DominQuery.h>
#include "CommucationOpTable.h"
#include "TaskTable.h"
#include "CommucationTaskInfoTable.h"
#include "NpuInfoTable.h"
using namespace Dic::Module::Timeline;
namespace Dic::TimeLine::HcclRepo::Mock {
void QueryCompeteSliceByIdsGroupTrackWithNormalExcuteQuery(const std::string &fileId,
    std::vector<CommucationTaskOpPO> &result)
{
    CommucationTaskOpPO commucationTaskOpPO1 = { 1, 22, 45, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 };
    CommucationTaskOpPO commucationTaskOpPO2 = { 2, 23, 50, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0 };
    result.emplace_back(commucationTaskOpPO1);
    result.emplace_back(commucationTaskOpPO2);
}

void QueryCompeteSliceByIdsPlaneTrackWithTaskTableMock(const std::string &fileId, std::vector<TaskPO> &result)
{
    TaskPO taskPO1 = { 1, 15, 33, 2, 0, 33, 0, 0, 0, 0, 0, 0 };
    TaskPO taskPO2 = { 2, 16, 33, 2, 0, 34, 0, 0, 0, 0, 0, 0 };
    result.emplace_back(taskPO1);
    result.emplace_back(taskPO2);
}

void QueryCompeteSliceByIdsPlaneTrackWithCommucationTaskInfoTableMock(const std::string &fileId,
    std::vector<CommucationTaskInfoPO> &result)
{
    CommucationTaskInfoPO taskPO1 = { 1, 0, 33, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    CommucationTaskInfoPO taskPO2 = { 2, 0, 34, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    result.emplace_back(taskPO1);
    result.emplace_back(taskPO2);
}

void QueryGlobalTaskIdsByRankWithTaskTableMock(const std::string &fileId, std::vector<TaskPO> &result)
{
    TaskPO taskPO1 = { 0, 0, 0, 0, 0, 33, 0, 0, 0, 0, 0, 0 };
    TaskPO taskPO2 = { 0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0 };
    result.emplace_back(taskPO1);
    result.emplace_back(taskPO2);
}

void QueryOpIdsByGlabalTaskIdsForCommucationTaskInfoTable(const std::string &fileId,
    std::vector<CommucationTaskInfoPO> &result)
{
    CommucationTaskInfoPO taskPO1 = { 1, 0, 33, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7 };
    CommucationTaskInfoPO taskPO2 = { 2, 0, 34, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8 };
    result.emplace_back(taskPO1);
    result.emplace_back(taskPO2);
}

void QuerySimpleSliceByIdsGroupTrackForCommucationOpTable(const std::string &fileId,
    std::vector<CommucationTaskOpPO> &result)
{
    CommucationTaskOpPO commucationTaskOpPO1 = { 1, 22, 45, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 };
    CommucationTaskOpPO commucationTaskOpPO2 = { 2, 23, 50, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0 };
    result.emplace_back(commucationTaskOpPO1);
    result.emplace_back(commucationTaskOpPO2);
}

void QueryAllPlaneSliceForTaskTableMock(const std::string &fileId, std::vector<TaskPO> &result)
{
    TaskPO taskPO1 = { 1, 28, 41, 0, 0, 33, 0, 0, 0, 0, 0, 0 };
    TaskPO taskPO2 = { 2, 29, 56, 0, 0, 34, 0, 0, 0, 0, 0, 0 };
    result.emplace_back(taskPO1);
    result.emplace_back(taskPO2);
}

void QueryUniqueDeviceIdForNpuInfoTableMock(const std::string &fileId, std::vector<NpuInfoPo> &result)
{
    NpuInfoPo po = {"device1", 0};
    result.push_back(po);
}
}
#endif // PROFILER_SERVER_HCCLREPO_MOCK_DATA_H
