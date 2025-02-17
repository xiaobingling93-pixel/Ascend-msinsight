/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MetaDataCacheManager.h"

using namespace Dic::Module::Timeline;
class MetaDataCacheManagerTest : public ::testing::Test {
};

TEST_F(MetaDataCacheManagerTest, test_MetaDataCacheManager_success)
{
    MetaDataCacheManager::Instance().Clear();
    std::vector<ParallelGroupInfo> groupInfoList;
    groupInfoList.push_back({"10.174.216.241%enp189s0f1_55000_0_1738895486152654", "default_group", {"1", "2"}});
    groupInfoList.push_back({"", "group", {"1", "2"}});
    MetaDataCacheManager::Instance().AddParallelGroupInfo(groupInfoList);
    EXPECT_EQ(MetaDataCacheManager::Instance().GetParallelGroupInfo("").has_value(), false);
    EXPECT_EQ(MetaDataCacheManager::Instance().GetParallelGroupInfo("test").has_value(), false);
    auto infoOpt = MetaDataCacheManager::Instance()
        .GetParallelGroupInfo("10.174.216.241%enp189s0f1_55000_0_1738895486152654");
    EXPECT_EQ(infoOpt.has_value(), true);
    EXPECT_EQ(infoOpt.value().groupName, "default_group");
    MetaDataCacheManager::Instance().Clear();
}