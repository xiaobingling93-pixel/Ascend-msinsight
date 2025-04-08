/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "CollectionTimeService.h"
#include "TestCaseDatabaseUtil.h"
using namespace Dic::Protocol;
using namespace Dic::TimeLine::TestCaseUtil;
class CollectionTimeServiceTest : public ::testing::Test {};

TEST_F(CollectionTimeServiceTest, TestComputeMarkHostWithHostPath)
{
    std::string host = "mshost";
    uint64_t startTime = 100;
    uint64_t endTime = 200;
    std::string hostPath1 = "D:/test/cluster_1/host";
    std::string hostPath2 = "D:/test/cluster_2/host";

    std::string host1 = CollectionTimeService::Instance().ComputeMarkHost(host, hostPath1, startTime, endTime);
    std::string host2 = CollectionTimeService::Instance().ComputeMarkHost(host, hostPath2, startTime, endTime);
    std::string host3 = CollectionTimeService::Instance().ComputeMarkHost(host, hostPath1, startTime, endTime);

    EXPECT_EQ(host1, "mshost_0 ");
    EXPECT_EQ(host2, "mshost_1 ");
    EXPECT_EQ(host3, "mshost_0 ");
}