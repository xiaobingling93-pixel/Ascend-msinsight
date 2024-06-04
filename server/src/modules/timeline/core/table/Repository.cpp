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
        .ExcuteQuery(sliceQuery.db, tempSlicePOVec);
    for (const auto &item : tempSlicePOVec) {
        SliceDomain cachelice;
        cachelice.id = item.id;
        cachelice.timestamp = item.timestamp;
        cachelice.endTime = item.endTime;
        sliceVec.emplace_back(cachelice);
    }
}

void Repository::QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::set<uint64_t> &sliceIds)
{
    SliceTable sliceTable;
    std::vector<SlicePO> slicePOVec;
    sliceTable.Select(SliceColumn::ID)
        .Eq(SliceColumn::TRACKID, sliceQuery.trackId)
        .Eq(SliceColumn::CAT, sliceQuery.cat)
        .ExcuteQuery(sliceQuery.db, slicePOVec);
    for (const auto &item : slicePOVec) {
        sliceIds.emplace(item.id);
    }
}
}
