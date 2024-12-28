/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SummaryProtocolRequest.h"

class SummaryProtocolRequestTest : public ::testing::Test {};

TEST_F(SummaryProtocolRequestTest, SummaryTopRankParamsTest)
{
    Dic::Protocol::SummaryTopRankParams params;
    params.orderBy = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, SummaryStatisticParamsTestTimeFlagInvaild)
{
    Dic::Protocol::SummaryStatisticParams params;
    params.timeFlag = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, SummaryStatisticParamsTestStepIdInvaild)
{
    Dic::Protocol::SummaryStatisticParams params;
    params.timeFlag = "time";
    params.stepId = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, PipelineStageTimeParamTeststageIdInvaild)
{
    Dic::Protocol::PipelineStageTimeParam params;
    params.stepId = "2";
    params.stageId = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, ComputeDetailParamsTestTimeFlagInvaild)
{
    Dic::Protocol::ComputeDetailParams params;
    params.timeFlag = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, ComputeDetailParamsTestOrderByInvaild)
{
    Dic::Protocol::ComputeDetailParams params;
    params.timeFlag = "time";
    params.orderBy = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, ComputeDetailParamsTestOrderInvaild)
{
    Dic::Protocol::ComputeDetailParams params;
    params.timeFlag = "time";
    params.orderBy = "orderBy";
    params.order = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, CommunicationDetailParamsTestRankIdInvaild)
{
    Dic::Protocol::ComputeDetailParams params;
    params.rankId = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, CommunicationDetailParamsTestTimeFlagInvaild)
{
    Dic::Protocol::ComputeDetailParams params;
    params.rankId = "1";
    params.timeFlag = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, CommunicationDetailParamsTestOrderByInvaild)
{
    Dic::Protocol::ComputeDetailParams params;
    params.rankId = "1";
    params.timeFlag = "time";
    params.orderBy = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}

TEST_F(SummaryProtocolRequestTest, CommunicationDetailParamsTestOrderInvaild)
{
    Dic::Protocol::ComputeDetailParams params;
    params.rankId = "1";
    params.timeFlag = "time";
    params.orderBy = "orderBy";
    params.order = ";";
    std::string msg;
    EXPECT_EQ(params.CheckParams(msg), false);
}