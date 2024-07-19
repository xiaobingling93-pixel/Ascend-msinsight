/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SLICETABLE_H
#define PROFILER_SERVER_SLICETABLE_H
#include "JsonTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct SlicePO {
    uint64_t id = 0;
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    uint64_t endTime = 0;
    uint64_t trackId = 0;
    std::string name;
    std::string args;
    std::string cat;
    std::string flagId;
    std::string cname;
};
class SliceTable : public Table<SlicePO> {
public:
    SliceTable() = default;
    ~SliceTable() = default;

protected:
    std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { SliceColumn::ID, IdHandle },
            { SliceColumn::TIMESTAMP, TimeStampHandle },
            { SliceColumn::DURATION, DurationHandle },
            { SliceColumn::NAME, NameHandle },
            { SliceColumn::TRACKID, TrackIdHandle },
            { SliceColumn::CAT, CatHandle },
            { SliceColumn::ARGS, ArgsHandle },
            { SliceColumn::CNAME, CnameHandle },
            { SliceColumn::ENDTIME, EndTimeHandle },
            { SliceColumn::FLAGID, FlagIdHandle } };

        return assignMap;
    }
    std::string &GetTableName() override
    {
        static std::string tableName = "slice";
        return tableName;
    }
    static void IdHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TimeStampHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void DurationHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void NameHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TrackIdHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void CatHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void ArgsHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void CnameHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void EndTimeHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void FlagIdHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}


#endif // PROFILER_SERVER_SLICETABLE_H
