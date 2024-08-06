/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_FLOWTABLE_H
#define PROFILER_SERVER_FLOWTABLE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct FlowPO {
    uint64_t id = 0;
    std::string flowId;
    std::string name;
    std::string cat;
    uint64_t trackId = 0;
    uint64_t timestamp = 0;
    std::string type;
};

class FlowTable : public Table<FlowPO> {
public:
    FlowTable() = default;
    ~FlowTable() = default;

protected:
    const std::unordered_map<std::string_view, assign>& GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { FlowColumn::ID, IdHandle },
            { FlowColumn::FLOW_ID, FlowIdHandle },
            { FlowColumn::NAME, NameHandle },
            { FlowColumn::CAT, CatHandle },
            { FlowColumn::TRACK_ID, TrackIdHandle },
            { FlowColumn::TIMESTAMP, TimeStampHandle },
            { FlowColumn::TYPE, TypeHandle } };

        return assignMap;
    }
    const std::string &GetTableName() override
    {
        static std::string tableName = "flow";
        return tableName;
    }
    static void IdHandle(FlowPO &flowPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void FlowIdHandle(FlowPO &flowPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void NameHandle(FlowPO &flowPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void CatHandle(FlowPO &flowPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TrackIdHandle(FlowPO &flowPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TimeStampHandle(FlowPO &flowPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TypeHandle(FlowPO &flowPO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_FLOWTABLE_H
