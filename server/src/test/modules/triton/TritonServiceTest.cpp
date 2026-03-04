/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include <gtest/gtest.h>
#include <vector>
#include <limits>
#include "TritonService.h"

using namespace Dic::Module::Triton;

class TritonServiceTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        TritonService::Instance().Reset();
    }
    
    void TearDown() override
    {
        TritonService::Instance().Reset();
    }
};

/**
 * @brief 场景说明：测试 TritonService 的基本数据更新和获取功能。
 */
TEST_F(TritonServiceTest, BasicUpdateAndGetTest)
{
    TritonMemeHeader header;
    header.kernelName = "test_kernel";
    TritonService::Instance().SetHeader(std::move(header));
    
    EXPECT_EQ(TritonService::Instance().GetHeader().kernelName, "test_kernel");
    
    std::vector<TritonTensorSegment> segments;
    TritonTensorSegment s1;
    s1.start = 100;
    s1.end = 200;
    segments.push_back(s1);
    
    TritonService::Instance().UpdateRecord(std::move(segments));
    auto result = TritonService::Instance().QuerySegmentsContainRange(150);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].start, 100);
}

/**
 * @brief 场景说明：测试 QuerySegmentsContainRange 在不同时间范围下的查询结果。
 * 验证包含、不包含以及边界情况。
 */
TEST_F(TritonServiceTest, QuerySegmentsContainRangeTest)
{
    std::vector<TritonTensorSegment> segments;
    TritonTensorSegment s1;
    s1.start = 100;
    s1.end = 500;
    segments.push_back(s1);
    
    TritonTensorSegment s2;
    s2.start = 200;
    s2.end = 400;
    segments.push_back(s2);
    
    TritonService::Instance().UpdateRecord(std::move(segments));
    
    // 场景1：查询范围完全包含在 s1 中，但不包含在 s2 中
    auto res1 = TritonService::Instance().QuerySegmentsContainRange(150);
    EXPECT_EQ(res1.size(), 1);
    EXPECT_EQ(res1[0].start, 100);
    
    // 场景2：查询范围包含在 s1 和 s2 中
    auto res2 = TritonService::Instance().QuerySegmentsContainRange(250);
    EXPECT_EQ(res2.size(), 2);
    
    // 场景3：查询范围超出了所有 segment
    auto res3 = TritonService::Instance().QuerySegmentsContainRange(50);
    EXPECT_EQ(res3.size(), 0);
    
    // 场景4：无效范围 (start > end)
    auto res4 = TritonService::Instance().QuerySegmentsContainRange(300);
    EXPECT_EQ(res4.size(), 0);
}

/**
 * @brief 场景说明：测试 QueryBlocksContainRange 功能。
 * 验证从 segment 中提取 block 的逻辑，以及 block 属性（如 sourceLocation）的正确传递。
 */
TEST_F(TritonServiceTest, QueryBlocksContainRangeTest)
{
    std::vector<TritonTensorSegment> segments;
    TritonTensorSegment s1;
    s1.start = 100;
    s1.end = 500;
    s1.sourceLocation = "loc1";
    s1.buffer = "buf1";
    
    TritonTensorBlock b1;
    b1.id = 1;
    s1.blocks.push_back(b1);
    segments.push_back(s1);
    
    TritonService::Instance().UpdateRecord(std::move(segments));
    
    // 场景1：查询包含该 block 的范围
    auto res1 = TritonService::Instance().QueryBlocksContainRange(150, 450);
    ASSERT_EQ(res1.size(), 1);
    EXPECT_EQ(res1[0].id, 1);
    EXPECT_EQ(res1[0].sourceLocation, "loc1");
    EXPECT_EQ(res1[0].buffer, "buf1");
    
    // 场景2：查询不包含该 block 的范围
    auto res2 = TritonService::Instance().QueryBlocksContainRange(50, 600);
    EXPECT_EQ(res2.size(), 0);
}
