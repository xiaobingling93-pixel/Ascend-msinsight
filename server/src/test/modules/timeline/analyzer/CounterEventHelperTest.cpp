/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "CounterEventHelper.h"
using namespace Dic::Module::Timeline;
class CounterEventHelperTest : public testing::Test {
};

TEST_F(CounterEventHelperTest, GenerateMetaDataSQLTest)
{
    CounterEventHelper helper;
    helper.RegisterHostMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::CPU_USAGE;
    std::string sql = helper.GenerateMetaDataSQL(type);
    const std::string cpuUsageSQL =
        "SELECT DISTINCT 'CPU ' || cpuId || '' AS name, 'usage(%)' AS types FROM CPU_USAGE;";
    EXPECT_EQ(sql, cpuUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_DISK_USAGE;
    sql = helper.GenerateMetaDataSQL(type);
    const std::string diskUsageSQL =
        "SELECT DISTINCT 'Disk Usage' AS name, 'usage(%)' AS types FROM HOST_DISK_USAGE;";
    EXPECT_EQ(sql, diskUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_MEM_USAGE;
    sql = helper.GenerateMetaDataSQL(type);
    const std::string memUsageSQL =
        "SELECT DISTINCT 'Memory Usage' AS name, 'usage(%)' AS types FROM HOST_MEM_USAGE;";
    EXPECT_EQ(sql, memUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_NETWORK_USAGE;
    sql = helper.GenerateMetaDataSQL(type);
    const std::string networkUsageSQL =
        "SELECT DISTINCT 'Network Usage' AS name, 'usage(%)' AS types FROM HOST_NETWORK_USAGE;";
    EXPECT_EQ(sql, networkUsageSQL);
}

TEST_F(CounterEventHelperTest, GenerateCounterSQLTest)
{
    CounterEventHelper helper;
    helper.RegisterHostMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::CPU_USAGE;
    std::string sql = helper.GenerateCounterSQL(type);
    const std::string cpuUsageSQL =
        "SELECT timestampNs - ? AS startTime, '{\"usage(%)\":' || usage || '}' AS args FROM CPU_USAGE"
        " WHERE 'CPU ' || cpuId || '' = ? AND startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, cpuUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_DISK_USAGE;
    sql = helper.GenerateCounterSQL(type);
    const std::string diskUsageSQL =
        "SELECT timestampNs - ? AS startTime, '{\"usage(%)\":' || usage || '}' AS args FROM HOST_DISK_USAGE"
        " WHERE 'Disk Usage' = ? AND startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, diskUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_MEM_USAGE;
    sql = helper.GenerateCounterSQL(type);
    const std::string memUsageSQL =
        "SELECT timestampNs - ? AS startTime, '{\"usage(%)\":' || usage || '}' AS args FROM HOST_MEM_USAGE"
        " WHERE 'Memory Usage' = ? AND startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, memUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_NETWORK_USAGE;
    sql = helper.GenerateCounterSQL(type);
    const std::string networkUsageSQL =
        "SELECT timestampNs - ? AS startTime, '{\"usage(%)\":' || usage || '}' AS args FROM HOST_NETWORK_USAGE"
        " WHERE 'Network Usage' = ? AND startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, networkUsageSQL);
}