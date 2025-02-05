/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "TaskTable.h"
#include "NpuInfoRepo.h"
#include "TrackInfoManager.h"
#include "DeviceFlowRepo.h"
#include "CommucationOpTable.h"
#include "../../../DatabaseTestCaseMockUtil.cpp"
using namespace Dic::Global::PROFILER::MockUtil;
using namespace Dic::Module::FullDb;
class DeviceFlowRepoTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        TrackInfoManager::Instance().Reset();
    }

    void TearDown() override
    {
        TrackInfoManager::Instance().Reset();
    }
};

DeviceFlowRepo GetDeviceFlowRepoMock()
{
    class TaskMock : public Dic::Module::Timeline::TaskTable {
    public:
        void ExcuteQuery(const std::string &fileId, std::vector<TaskPO> &result) override
        {
            TaskPO taskPO1 = { 0, 0, 0, 0, 0, 33, 0, 0, 0, 0, 0, 0 };
            result.emplace_back(taskPO1);
            ClearThreadLocal();
        }
    };
    class CommucationOpTableMock : public Dic::Module::Timeline::CommucationOpTable {
    public:
        void ExcuteQuery(const std::string &fileId, std::vector<CommucationTaskOpPO> &result) override
        {
            CommucationTaskOpPO commucationTaskOpPO1 = { 1, 22, 45, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 };
            result.emplace_back(commucationTaskOpPO1);
            ClearThreadLocal();
        }
    };
    class NpuInfoTableMock : public Dic::Module::Timeline::NpuInfoTable {
    public:
        void ExcuteQuery(const std::string &fileId, std::vector<NpuInfoPo> &result) override
        {
            NpuInfoPo po = {"device1", 0};
            result.push_back(po);
            ClearThreadLocal();
        }
    };
    std::unique_ptr<Dic::Module::Timeline::TaskTable> tPtr = std::make_unique<TaskMock>();
    std::unique_ptr<Dic::Module::Timeline::CommucationOpTable> copPtr = std::make_unique<CommucationOpTableMock>();
    std::unique_ptr<Dic::Module::Timeline::NpuInfoTable> niPtr = std::make_unique<NpuInfoTableMock>();
    DeviceFlowRepo deviceFlowRepo;
    deviceFlowRepo.SetTaskTable(std::move(tPtr));
    deviceFlowRepo.SetCommucationOpTable(std::move(copPtr));
    std::unique_ptr<NpuInfoRepo> npr = std::make_unique<NpuInfoRepo>();
    npr->SetNpuInfoTable(std::move(niPtr));
    deviceFlowRepo.SetNpuInfoRepo(std::move(npr));
    return deviceFlowRepo;
}

TEST_F(DeviceFlowRepoTest, test_AddDeviceFlowPoint)
{
    DeviceFlowRepo deviceFlowRepo = GetDeviceFlowRepoMock();
    std::vector<FlowPoint> flowPointVec;
    FlowQuery flowQuery;
    deviceFlowRepo.AddDeviceFlowPoint(flowQuery, flowPointVec);
    int expectCount = 1;
    EXPECT_EQ(flowPointVec.size(), expectCount);
}