/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "../../TestSuit.cpp"
#include "ProtocolDefs.h"
#include "AclnnOpAdvisor.h"
#include "AffinityAPIAdvisor.h"
#include "AffinityOptimizerAdvisor.h"
#include "AICpuOpAdvisor.h"
#include "FusedOpAdvisor.h"
#include "OperatorDispatchAdvisor.h"
#include "DataBaseManager.h"

using namespace Dic::Protocol;
using namespace Dic::Module::Advisor;
using namespace Dic::Module::Timeline;

class AdvisorProcessTest : public TestSuit {
protected:
    APITypeParams params = {
        .rankId = "0",
        .currentPage = 2,
        .pageSize = 20, // 20 record per page, and now page 2
        .orderBy = "duration",
        .orderType = "ascend"
    };
};

TEST_F(AdvisorProcessTest, AclnnOpAdvisorProcessSuccessfulWithSpecificParam)
{
    Dic::Module::Timeline::DataBaseManager::Instance().SetDataType(DataType::TEXT);

    AclnnOperatorResBody resBody = {};
    auto res = AclnnOpAdvisor::Process(params, resBody);
    EXPECT_EQ(res, true);
    EXPECT_EQ(resBody.size, 0);
}

TEST_F(AdvisorProcessTest, AffinityAPIAdvisorProcessSuccessfulWithSpecificParam)
{
    Dic::Module::Timeline::DataBaseManager::Instance().SetDataType(DataType::TEXT);

    AffinityAPIResBody resBody = {};
    auto res = AffinityAPIAdvisor::Process(params, resBody);
    EXPECT_EQ(res, true);
    EXPECT_EQ(resBody.size, 24); // There are 24 that meet the criteria
    EXPECT_EQ(resBody.datas.size(), 4); // There are 4 that in page 2
}

TEST_F(AdvisorProcessTest, AffinityOptimizerAdvisorProcessSuccessfulWithSpecificParam)
{
    Dic::Module::Timeline::DataBaseManager::Instance().SetDataType(DataType::TEXT);

    AffinityOptimizerResBody resBody = {};
    auto res = AffinityOptimizerAdvisor::Process(params, resBody);
    EXPECT_EQ(res, true);
    EXPECT_EQ(resBody.size, 1);
}

TEST_F(AdvisorProcessTest, AICpuOpAdvisorProcessSuccessfulWithSpecificParam)
{
    Dic::Module::Timeline::DataBaseManager::Instance().SetDataType(DataType::TEXT);

    AICpuOperatorResBody resBody = {};
    auto res = AICpuOpAdvisor::Process(params, resBody);
    EXPECT_EQ(res, true);
    EXPECT_EQ(resBody.size, 2); // The size of resBody is 2
}

TEST_F(AdvisorProcessTest, FusedOpAdvisorProcessSuccessfulWithSpecificParam)
{
    Dic::Module::Timeline::DataBaseManager::Instance().SetDataType(DataType::TEXT);

    OperatorFusionResBody resBody = {};
    auto res = FusedOpAdvisor::Process(params, resBody);
    EXPECT_EQ(res, true);
    EXPECT_EQ(resBody.size, 0);
}

TEST_F(AdvisorProcessTest, OperatorDispatchAdvisorProcessSuccessfulWithSpecificParam)
{
    Dic::Module::Timeline::DataBaseManager::Instance().SetDataType(DataType::TEXT);

    OperatorDispatchResBody resBody = {};
    auto res = OperatorDispatchAdvisor::Process(params, resBody);
    EXPECT_EQ(res, true);
    EXPECT_EQ(resBody.size, 529); // The size of resBody is 529
}
