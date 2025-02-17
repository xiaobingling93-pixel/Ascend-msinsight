/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "../../TestSuit.cpp"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "AffinityAPIAdvisor.h"
#include "AffinityOptimizerAdvisor.h"
#include "AclnnOpAdvisor.h"
#include "AICpuOpAdvisor.h"
#include "FusedOpAdvisor.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::Advisor;
class TextAdvisorTest : TestSuit {
};

TEST_F(TestSuit, QueryAffinityApiAdvisorSuccessInText)
{
    DataBaseManager::Instance().SetDataType(DataType::TEXT);
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EXPECT_NE(db, nullptr);
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    Protocol::KernelDetailsParams params = {"duration", "DESC", 1, 10}; // 1是第1页，10是每页10条数据
    std::map<uint64_t, std::vector<Protocol::FlowLocation>> dataMap{};
    std::map<uint64_t, std::vector<uint32_t>> indexMap{};
    auto result = db->QueryAffinityAPIData(params, {"aten::reshape"}, startTime, dataMap, indexMap);
    const uint64_t expectTrackId = 65;
    EXPECT_TRUE(result);
    EXPECT_EQ(dataMap.size(), 1); // 符合结果的包含1个泳道
    EXPECT_EQ(dataMap.begin()->first, expectTrackId); // 符合结果的包含1个泳道，且track_id = 41
    EXPECT_GT(dataMap.at(expectTrackId).size(), 1); // 符合条件的结果为1条
    EXPECT_EQ(indexMap.size(), 1);
    EXPECT_EQ(indexMap.begin()->first, expectTrackId); // 符合结果的包含1个泳道，且track_id = 41
    EXPECT_EQ(indexMap.at(expectTrackId).size(), 24); // 符合结果的包含1个泳道，且track_id = 41，符合条件的结果为24条
}

TEST_F(TestSuit, QueryAffinityOptimizerAdvisorSuccessText)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EXPECT_NE(db, nullptr);
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    Protocol::KernelDetailsParams params = {"duration", "DESC", 1, 10}; // 1是第1页，10是每页10条数据
    std::vector<Protocol::ThreadTraces> data{};
    auto result = db->QueryAffinityOptimizer(params, "'Optimizer.step#AdamW.step'", data, startTime);
    EXPECT_TRUE(result);
    EXPECT_EQ(data.size(), 1); //
    EXPECT_EQ(data.at(0).name, "Optimizer.step#AdamW.step"); //
}

TEST_F(TestSuit, QueryAclNNOperatorAdvisorSuccessText)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EXPECT_NE(db, nullptr);
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    std::vector<Protocol::KernelBaseInfo> data{};
    Protocol::KernelDetailsParams params = {"duration", "DESC", 1, 10}; // 1是第1页，10是每页10条数据
    auto result = db->QueryAclnnOpCountExceedThreshold(params, ACLNN_OP_CNT_THRESHOLD, data, startTime);
    EXPECT_TRUE(result);
    EXPECT_EQ(data.size(), 0);
}

TEST_F(TestSuit, QueryAICPUOperatorAdvisorSuccessText)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EXPECT_NE(db, nullptr);
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    std::vector<Protocol::KernelBaseInfo> data{};
    Protocol::KernelDetailsParams params = {"duration", "DESC", 1, 10}; // 1是第1页，10是每页10条数据
    auto result = db->QueryAICpuOpCanBeOptimized(params,
        AICPU_OP_EQUIVALENT_REPLACE, AICPU_OP_DATATYPE_RULE, data, startTime);
    EXPECT_TRUE(result);
    EXPECT_EQ(data.size(), 2); // 可以查询到2条结果
}

TEST_F(TestSuit, QueryFusedOperatorAdvisorSuccessText)
{
    auto db = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    EXPECT_NE(db, nullptr);
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    Protocol::KernelDetailsParams params = {"duration", "DESC", 1, 10}; // 1是第1页，10是每页10条数据
    std::vector<Protocol::FlowLocation> data{};
    auto result = db->QueryFuseableOpData(params, FUSEABLE_OPERATER_RULE_LIST.at(0), data, startTime);
    EXPECT_TRUE(result);
    EXPECT_EQ(data.size(), 0);
}