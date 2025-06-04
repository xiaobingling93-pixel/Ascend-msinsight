/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "CounterEventHelper.h"
using namespace Dic::Module::Timeline;
class CounterEventHelperTest : public testing::Test {
};

TEST_F(CounterEventHelperTest, GenerateHostMetaDataSQLTest)
{
    CounterEventHelper helper;
    helper.RegisterHostMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::CPU_USAGE;
    std::string sql = helper.GenerateHostMetadataSQL(type);
    const std::string cpuUsageSQL =
        "SELECT DISTINCT 'CPU ' || cpuId || '' AS name, 'usage(%)' AS types FROM CPU_USAGE;";
    EXPECT_EQ(sql, cpuUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_DISK_USAGE;
    sql = helper.GenerateHostMetadataSQL(type);
    const std::string diskUsageSQL =
        "SELECT DISTINCT 'Disk Usage' AS name, 'usage(%)' AS types FROM HOST_DISK_USAGE;";
    EXPECT_EQ(sql, diskUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_MEM_USAGE;
    sql = helper.GenerateHostMetadataSQL(type);
    const std::string memUsageSQL =
        "SELECT DISTINCT 'Memory Usage' AS name, 'usage(%)' AS types FROM HOST_MEM_USAGE;";
    EXPECT_EQ(sql, memUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_NETWORK_USAGE;
    sql = helper.GenerateHostMetadataSQL(type);
    const std::string networkUsageSQL =
        "SELECT DISTINCT 'Network Usage' AS name, 'usage(%)' AS types FROM HOST_NETWORK_USAGE;";
    EXPECT_EQ(sql, networkUsageSQL);
}

TEST_F(CounterEventHelperTest, GenerateHostCounterSQLTest)
{
    CounterEventHelper helper;
    helper.RegisterHostMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::CPU_USAGE;
    std::string sql = helper.GenerateHostCounterSQL(type);
    const std::string cpuUsageSQL =
        "SELECT timestampNs - ? AS startTime, '{\"usage(%)\":' || usage || '}' AS args FROM CPU_USAGE"
        " WHERE 'CPU ' || cpuId || '' = ? AND startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, cpuUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_DISK_USAGE;
    sql = helper.GenerateHostCounterSQL(type);
    const std::string diskUsageSQL =
        "SELECT timestampNs - ? AS startTime, '{\"usage(%)\":' || usage || '}' AS args FROM HOST_DISK_USAGE"
        " WHERE 'Disk Usage' = ? AND startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, diskUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_MEM_USAGE;
    sql = helper.GenerateHostCounterSQL(type);
    const std::string memUsageSQL =
        "SELECT timestampNs - ? AS startTime, '{\"usage(%)\":' || usage || '}' AS args FROM HOST_MEM_USAGE"
        " WHERE 'Memory Usage' = ? AND startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, memUsageSQL);
    type = Dic::Protocol::PROCESS_TYPE::HOST_NETWORK_USAGE;
    sql = helper.GenerateHostCounterSQL(type);
    const std::string networkUsageSQL =
        "SELECT timestampNs - ? AS startTime, '{\"usage(%)\":' || usage || '}' AS args FROM HOST_NETWORK_USAGE"
        " WHERE 'Network Usage' = ? AND startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, networkUsageSQL);
}

TEST_F(CounterEventHelperTest, GenerateDeviceMetaDataSQLForNICTest)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::NIC;
    std::string sql = helper.GenerateDeviceMetadataSQL(type);
    const std::string nicSQL =
        "SELECT DISTINCT 'NIC/macTxPfcPkt' AS name, 'Pkt' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/macRxPfcPkt' AS name, 'Pkt' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/macTxByte' AS name, 'Byte' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/macTxBandwidth' AS name, 'Bandwidth(B/s)' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/macRxByte' AS name, 'Byte' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/macRxBandwidth' AS name, 'Bandwidth(B/s)' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/macTxBadByte' AS name, 'Byte' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/macRxBadByte' AS name, 'Byte' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/nicTxByte' AS name, 'Byte' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/nicTxBandwidth' AS name, 'Bandwidth(B/s)' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/nicRxByte' AS name, 'Byte' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'NIC/nicRxBandwidth' AS name, 'Bandwidth(B/s)' AS types FROM NETDEV_STATS;";
    EXPECT_EQ(sql, nicSQL);
}

TEST_F(CounterEventHelperTest, GenerateDeviceCounterSQLForNICTestPartOne)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::NIC;
    std::string threadId = "NIC/macTxPfcPkt";
    std::string sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL1 =
        "SELECT timestampNs - ? AS startTime, '{\"Pkt\":' || macTxPfcPkt || '}' AS args FROM NETDEV_STATS "
        "WHERE 'NIC/macTxPfcPkt' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL1);
    threadId = "NIC/macRxPfcPkt";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL2 =
        "SELECT timestampNs - ? AS startTime, '{\"Pkt\":' || macRxPfcPkt || '}' AS args FROM NETDEV_STATS "
        "WHERE 'NIC/macRxPfcPkt' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL2);
    threadId = "NIC/macTxByte";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL3 =
        "SELECT timestampNs - ? AS startTime, '{\"Byte\":' || macTxByte || '}' AS args FROM NETDEV_STATS "
        "WHERE 'NIC/macTxByte' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL3);
    threadId = "NIC/macTxBandwidth";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL4 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || macTxBandwidth || '}' AS args "
        "FROM NETDEV_STATS WHERE 'NIC/macTxBandwidth' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? "
        "ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL4);
    threadId = "NIC/macRxByte";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL5 =
        "SELECT timestampNs - ? AS startTime, '{\"Byte\":' || macRxByte || '}' AS args FROM NETDEV_STATS "
        "WHERE 'NIC/macRxByte' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL5);
    threadId = "NIC/macRxBandwidth";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL6 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || macRxBandwidth || '}' AS args "
        "FROM NETDEV_STATS WHERE 'NIC/macRxBandwidth' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? "
        "ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL6);
}

TEST_F(CounterEventHelperTest, GenerateDeviceCounterSQLForNICTestPartTwo)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::NIC;
    std::string threadId = "NIC/macTxBadByte";
    std::string sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL1 =
        "SELECT timestampNs - ? AS startTime, '{\"Byte\":' || macTxBadByte || '}' AS args FROM NETDEV_STATS "
        "WHERE 'NIC/macTxBadByte' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL1);
    threadId = "NIC/macRxBadByte";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL2 =
        "SELECT timestampNs - ? AS startTime, '{\"Byte\":' || macRxBadByte || '}' AS args FROM NETDEV_STATS "
        "WHERE 'NIC/macRxBadByte' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL2);
    threadId = "NIC/nicTxByte";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL3 =
        "SELECT timestampNs - ? AS startTime, '{\"Byte\":' || nicTxByte || '}' AS args FROM NETDEV_STATS "
        "WHERE 'NIC/nicTxByte' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL3);
    threadId = "NIC/nicTxBandwidth";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL4 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || nicTxBandwidth || '}' AS args "
        "FROM NETDEV_STATS WHERE 'NIC/nicTxBandwidth' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? "
        "ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL4);
    threadId = "NIC/nicRxByte";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL5 =
        "SELECT timestampNs - ? AS startTime, '{\"Byte\":' || nicRxByte || '}' AS args FROM NETDEV_STATS "
        "WHERE 'NIC/nicRxByte' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL5);
    threadId = "NIC/nicRxBandwidth";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string nicSQL6 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || nicRxBandwidth || '}' AS args "
        "FROM NETDEV_STATS WHERE 'NIC/nicRxBandwidth' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? "
        "ORDER BY startTime ASC;";
    EXPECT_EQ(sql, nicSQL6);
}

TEST_F(CounterEventHelperTest, GenerateDeviceMetaDataSQLForRoCETest)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::ROCE;
    std::string sql = helper.GenerateDeviceMetadataSQL(type);
    const std::string roceSQL =
        "SELECT DISTINCT 'RoCE/roceTxPkt' AS name, 'Pkt' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'RoCE/roceRxPkt' AS name, 'Pkt' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'RoCE/roceTxErrPkt' AS name, 'Pkt' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'RoCE/roceRxErrPkt' AS name, 'Pkt' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'RoCE/roceTxCnpPkt' AS name, 'Pkt' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'RoCE/roceRxCnpPkt' AS name, 'Pkt' AS types FROM NETDEV_STATS UNION ALL "
        "SELECT DISTINCT 'RoCE/roceNewPktRty' AS name, 'Rty' AS types FROM NETDEV_STATS;";
    EXPECT_EQ(sql, roceSQL);
}

TEST_F(CounterEventHelperTest, GenerateDeviceCounterSQLForRoCETest)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::ROCE;
    std::string threadId = "RoCE/roceTxPkt";
    std::string sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string roceSQL1 =
        "SELECT timestampNs - ? AS startTime, '{\"Pkt\":' || roceTxPkt || '}' AS args FROM NETDEV_STATS "
        "WHERE 'RoCE/roceTxPkt' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, roceSQL1);
    threadId = "RoCE/roceRxPkt";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string roceSQL2 =
        "SELECT timestampNs - ? AS startTime, '{\"Pkt\":' || roceRxPkt || '}' AS args FROM NETDEV_STATS "
        "WHERE 'RoCE/roceRxPkt' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, roceSQL2);
    threadId = "RoCE/roceTxErrPkt";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string roceSQL3 =
        "SELECT timestampNs - ? AS startTime, '{\"Pkt\":' || roceTxErrPkt || '}' AS args FROM NETDEV_STATS "
        "WHERE 'RoCE/roceTxErrPkt' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, roceSQL3);
    threadId = "RoCE/roceRxErrPkt";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string roceSQL4 =
        "SELECT timestampNs - ? AS startTime, '{\"Pkt\":' || roceRxErrPkt || '}' AS args FROM NETDEV_STATS "
        "WHERE 'RoCE/roceRxErrPkt' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, roceSQL4);
    threadId = "RoCE/roceTxCnpPkt";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string roceSQL5 =
        "SELECT timestampNs - ? AS startTime, '{\"Pkt\":' || roceTxCnpPkt || '}' AS args FROM NETDEV_STATS "
        "WHERE 'RoCE/roceTxCnpPkt' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, roceSQL5);
    threadId = "RoCE/roceRxCnpPkt";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string roceSQL6 =
        "SELECT timestampNs - ? AS startTime, '{\"Pkt\":' || roceRxCnpPkt || '}' AS args FROM NETDEV_STATS "
        "WHERE 'RoCE/roceRxCnpPkt' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, roceSQL6);
    threadId = "RoCE/roceNewPktRty";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string roceSQL7 =
        "SELECT timestampNs - ? AS startTime, '{\"Rty\":' || roceNewPktRty || '}' AS args FROM NETDEV_STATS "
        "WHERE 'RoCE/roceNewPktRty' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, roceSQL7);
}

TEST_F(CounterEventHelperTest, GenerateDeviceMetaDataSQLForPCIeTest)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::PCIE;
    std::string sql = helper.GenerateDeviceMetadataSQL(type);
    const std::string pcieSQL =
        "SELECT DISTINCT 'PCIE/txPostMin' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txPostMax' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txPostAvg' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/rxPostMin' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/rxPostMax' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/rxPostAvg' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txNonpostMin' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txNonpostMax' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txNonpostAvg' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/rxNonpostMin' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/rxNonpostMax' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/rxNonpostAvg' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txCplMin' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txCplMax' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txCplAvg' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/rxCplMin' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/rxCplMax' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/rxCplAvg' AS name, 'Bandwidth(B/s)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txNonpostLatencyMin' AS name, 'Time(ns)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txNonpostLatencyMax' AS name, 'Time(ns)' AS types FROM PCIE UNION ALL "
        "SELECT DISTINCT 'PCIE/txNonpostLatencyAvg' AS name, 'Time(ns)' AS types FROM PCIE;";
    EXPECT_EQ(sql, pcieSQL);
}

TEST_F(CounterEventHelperTest, GenerateDeviceCounterSQLForPCIeTestPartOne)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::PCIE;
    std::string threadId = "PCIE/txPostMin";
    std::string sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL1 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txPostMin || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txPostMin' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL1);
    threadId = "PCIE/txPostMax";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL2 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txPostMax || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txPostMax' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL2);
    threadId = "PCIE/txPostAvg";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL3 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txPostAvg || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txPostAvg' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL3);
    threadId = "PCIE/rxPostMin";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL4 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxPostMin || '}' AS args FROM PCIE "
        "WHERE 'PCIE/rxPostMin' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL4);
    threadId = "PCIE/rxPostMax";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL5 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxPostMax || '}' AS args FROM PCIE "
        "WHERE 'PCIE/rxPostMax' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL5);
    threadId = "PCIE/rxPostAvg";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL6 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxPostAvg || '}' AS args FROM PCIE "
        "WHERE 'PCIE/rxPostAvg' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL6);
}

TEST_F(CounterEventHelperTest, GenerateDeviceCounterSQLForPCIeTestPartTwo)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::PCIE;
    std::string threadId = "PCIE/txNonpostMin";
    std::string sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL1 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txNonpostMin || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txNonpostMin' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL1);
    threadId = "PCIE/txNonpostMax";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL2 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txNonpostMax || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txNonpostMax' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL2);
    threadId = "PCIE/txNonpostAvg";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL3 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txNonpostAvg || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txNonpostAvg' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL3);
    threadId = "PCIE/rxNonpostMin";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL4 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxNonpostMin || '}' AS args FROM PCIE "
        "WHERE 'PCIE/rxNonpostMin' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL4);
    threadId = "PCIE/rxNonpostMax";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL5 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxNonpostMax || '}' AS args FROM PCIE "
        "WHERE 'PCIE/rxNonpostMax' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL5);
    threadId = "PCIE/rxNonpostAvg";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL6 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxNonpostAvg || '}' AS args FROM PCIE "
        "WHERE 'PCIE/rxNonpostAvg' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL6);
}

TEST_F(CounterEventHelperTest, GenerateDeviceCounterSQLForPCIeTestPartThree)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::PCIE;
    std::string threadId = "PCIE/txCplMin";
    std::string sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL1 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txCplMin || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txCplMin' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL1);
    threadId = "PCIE/txCplMax";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL2 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txCplMax || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txCplMax' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL2);
    threadId = "PCIE/txCplAvg";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL3 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txCplAvg || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txCplAvg' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL3);
    threadId = "PCIE/rxCplMin";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL4 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxCplMin || '}' AS args FROM PCIE "
        "WHERE 'PCIE/rxCplMin' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL4);
    threadId = "PCIE/rxCplMax";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL5 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxCplMax || '}' AS args FROM PCIE "
        "WHERE 'PCIE/rxCplMax' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL5);
    threadId = "PCIE/rxCplAvg";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL6 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxCplAvg || '}' AS args FROM PCIE "
        "WHERE 'PCIE/rxCplAvg' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL6);
}

TEST_F(CounterEventHelperTest, GenerateDeviceCounterSQLForPCIeTestPartFour)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::PCIE;
    std::string threadId = "PCIE/txNonpostLatencyMin";
    std::string sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL1 =
        "SELECT timestampNs - ? AS startTime, '{\"Time(ns)\":' || txNonpostLatencyMin || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txNonpostLatencyMin' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? "
        "ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL1);
    threadId = "PCIE/txNonpostLatencyMax";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL2 =
        "SELECT timestampNs - ? AS startTime, '{\"Time(ns)\":' || txNonpostLatencyMax || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txNonpostLatencyMax' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? "
        "ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL2);
    threadId = "PCIE/txNonpostLatencyAvg";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string pcieSQL3 =
        "SELECT timestampNs - ? AS startTime, '{\"Time(ns)\":' || txNonpostLatencyAvg || '}' AS args FROM PCIE "
        "WHERE 'PCIE/txNonpostLatencyAvg' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? "
        "ORDER BY startTime ASC;";
    EXPECT_EQ(sql, pcieSQL3);
}

TEST_F(CounterEventHelperTest, GenerateDeviceMetaDataSQLForHCCSTest)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::HCCS;
    std::string sql = helper.GenerateDeviceMetadataSQL(type);
    const std::string roceSQL =
        "SELECT DISTINCT 'HCCS/txThroughput' AS name, 'Bandwidth(B/s)' AS types FROM HCCS UNION ALL "
        "SELECT DISTINCT 'HCCS/rxThroughput' AS name, 'Bandwidth(B/s)' AS types FROM HCCS;";
    EXPECT_EQ(sql, roceSQL);
}

TEST_F(CounterEventHelperTest, GenerateDeviceCounterSQLForHCCSTest)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    Dic::Protocol::PROCESS_TYPE type = Dic::Protocol::PROCESS_TYPE::HCCS;
    std::string threadId = "HCCS/txThroughput";
    std::string sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string hccsSQL1 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || txThroughput || '}' AS args FROM HCCS "
        "WHERE 'HCCS/txThroughput' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, hccsSQL1);
    threadId = "HCCS/rxThroughput";
    sql = helper.GenerateDeviceCounterSQL(type, threadId);
    const std::string hccsSQL2 =
        "SELECT timestampNs - ? AS startTime, '{\"Bandwidth(B/s)\":' || rxThroughput || '}' AS args FROM HCCS "
        "WHERE 'HCCS/rxThroughput' = ? AND startTime >= ? AND startTime <= ? AND deviceId = ? ORDER BY startTime ASC;";
    EXPECT_EQ(sql, hccsSQL2);
}