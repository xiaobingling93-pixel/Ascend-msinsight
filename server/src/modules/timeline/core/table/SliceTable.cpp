/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "SliceTable.h"
namespace Dic {
namespace Module {
namespace Timeline {
void SliceTable::IdHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.id = resultSet->GetUint64(SliceColumn::ID);
}
void SliceTable::TimeStampHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.timestamp = resultSet->GetUint64(SliceColumn::TIMESTAMP);
}
void SliceTable::EndTimeHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.endTime = resultSet->GetUint64(SliceColumn::ENDTIME);
}
void SliceTable::DurationHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.duration = resultSet->GetUint64(SliceColumn::DURATION);
}
void SliceTable::NameHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.name = resultSet->GetString(SliceColumn::NAME);
}
void SliceTable::CatHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.cat = resultSet->GetString(SliceColumn::CAT);
}
void SliceTable::ArgsHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.args = resultSet->GetString(SliceColumn::ARGS);
}
void SliceTable::CnameHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.cname = resultSet->GetString(SliceColumn::CNAME);
}
void SliceTable::TrackIdHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.trackId = resultSet->GetUint64(SliceColumn::TRACKID);
}
void SliceTable::FlagIdHandle(SlicePO &slicePo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    slicePo.flagId = resultSet->GetString(SliceColumn::FLAGID);
}
}
}
}
