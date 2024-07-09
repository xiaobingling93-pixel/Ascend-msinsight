/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "SliceTable.h"
#include "Repository.h"
namespace Dic::Module::Timeline {
void Repository::QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery, std::vector<SliceDomain> &sliceVec)
{
    SliceTable sliceTable;
    std::vector<SlicePO> tempSlicePOVec;
    sliceTable.Select(SliceColumn::ID, SliceColumn::TIMESTAMP, SliceColumn::ENDTIME)
        .Eq(SliceColumn::TRACKID, sliceQuery.trackId)
        .OrderBy(SliceColumn::TIMESTAMP, TableOrder::ASC)
        .OrderBy(SliceColumn::ID, TableOrder::ASC)
        .ExcuteQuery(sliceQuery.db, tempSlicePOVec);
    for (const auto &item : tempSlicePOVec) {
        SliceDomain cachelice;
        cachelice.id = item.id;
        cachelice.timestamp = item.timestamp;
        cachelice.endTime = item.endTime;
        sliceVec.emplace_back(cachelice);
    }
}

void Repository::QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds)
{
    SliceTable sliceTable;
    std::vector<SlicePO> slicePOVec;
    sliceTable.Select(SliceColumn::ID)
        .Eq(SliceColumn::TRACKID, sliceQuery.trackId)
        .Eq(SliceColumn::CAT, sliceQuery.cat)
        .OrderBy(SliceColumn::ID, TableOrder::ASC)
        .ExcuteQuery(sliceQuery.db, slicePOVec);
    for (const auto &item : slicePOVec) {
        sliceIds.emplace_back(item.id);
    }
}

uint64_t Repository::QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery)
{
    SliceTable sliceTable;
    uint64_t count = sliceTable.Eq(SliceColumn::TRACKID, sliceQuery.trackId)
                         .Eq(SliceColumn::CAT, sliceQuery.cat)
                         .Count(sliceQuery.db);
    return count;
}

void Repository::QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
    std::vector<CompeteSliceDomain> &sliceVec)
{
    SliceTable sliceTable;
    std::vector<SlicePO> slicePOVec;
    sliceTable.Select(SliceColumn::ID, SliceColumn::TIMESTAMP)
        .Select(SliceColumn::DURATION, SliceColumn::ENDTIME, SliceColumn::NAME)
        .Eq(SliceColumn::TRACKID, sliceQuery.trackId)
        .LessEq(SliceColumn::TIMESTAMP, sliceQuery.endTime + sliceQuery.minTimestamp)
        .Greater(SliceColumn::ENDTIME, sliceQuery.startTime + sliceQuery.minTimestamp)
        .ExcuteQuery(sliceQuery.db, slicePOVec);
    for (const auto &item : slicePOVec) {
        CompeteSliceDomain temp;
        temp.id = item.id;
        temp.timestamp = item.timestamp;
        temp.duration = item.duration;
        temp.endTime = item.endTime;
        temp.name = item.name;
        sliceVec.emplace_back(std::move(temp));
    }
}
}
