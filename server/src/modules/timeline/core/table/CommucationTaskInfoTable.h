/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUCATIONTASKINFO_H
#define PROFILER_SERVER_COMMUCATIONTASKINFO_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct CommucationTaskInfoPO {
    uint64_t id = 0;
    uint64_t name = 0;
    uint64_t globalTaskId = 0;
    uint64_t taskType = 0;
    uint64_t planeId = 0;
    uint64_t groupName = 0;
    uint64_t notifyId = 0;
    uint64_t rdmaType = 0;
    uint64_t srcRank = 0;
    uint64_t dstRank = 0;
    uint64_t transportType = 0;
    uint64_t size = 0;
    uint64_t dataType = 0;
    uint64_t linkType = 0;
    double bandwidth = 0;
    uint64_t opId = 0;
};
class CommucationTaskInfoTable : public Table<CommucationTaskInfoPO> {
public:
    CommucationTaskInfoTable() = default;
    ~CommucationTaskInfoTable() = default;

protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { CommucationTaskInfoColumn::ROW_ID, IdHandle },
            { CommucationTaskInfoColumn::NAME, NameHandle },
            { CommucationTaskInfoColumn::GLOBAL_TASK_ID, GlobalTaskIdHandle },
            { CommucationTaskInfoColumn::TASK_TYPE, TaskTypeHandle },
            { CommucationTaskInfoColumn::PLANE_ID, PlaneIdHandle },
            { CommucationTaskInfoColumn::GROUPNAME, GroupNameHandle },
            { CommucationTaskInfoColumn::NOTIFY_ID, NotifyIdHandle },
            { CommucationTaskInfoColumn::RDMA_TYPE, RdmaTypeHandle },
            { CommucationTaskInfoColumn::SRC_RANK, SrcRankHandle },
            { CommucationTaskInfoColumn::DST_RANK, DstRankHandle },
            { CommucationTaskInfoColumn::TRANSPORT_TYPE, TransportTypeHandle },
            { CommucationTaskInfoColumn::SIZE, SizeHandle },
            { CommucationTaskInfoColumn::DATA_TYPE, DataTypeHandle },
            { CommucationTaskInfoColumn::LINK_TYPE, LinkTypeHandle },
            { CommucationTaskInfoColumn::BANDWIDTH, BandwidthHandle },
            { CommucationTaskInfoColumn::OP_ID, OpIdHandle } };

        return assignMap;
    }
    const std::string &GetTableName() override
    {
        static std::string tableName = "COMMUNICATION_TASK_INFO";
        return tableName;
    }
    static void IdHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void NameHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void GlobalTaskIdHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TaskTypeHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void PlaneIdHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void GroupNameHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void NotifyIdHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void RdmaTypeHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SrcRankHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void DstRankHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TransportTypeHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SizeHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void DataTypeHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void LinkTypeHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void OpIdHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void BandwidthHandle(CommucationTaskInfoPO &commucationTaskInfoPO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_COMMUCATIONTASKINFO_H
