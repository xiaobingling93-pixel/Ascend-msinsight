/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "CannApiRepo_mock_data.h"
#include "CannApiRepo.h"
#include "TrackInfoManager.h"
using namespace Dic::TimeLine::CannApiRepo::Mock;
class CannApiRepoTest : public ::testing::Test {};

/**
 * 测试全量DB的cann根据id空集合查询完整算子
 */
TEST_F(CannApiRepoTest, test_QueryCompeteSliceByIds_with_empthIds)
{
    CannApiRepo cannApiRepo;
    SliceQuery sliceQuery;
    std::vector<uint64_t> sliceIds;
    std::vector<CompeteSliceDomain> competeSliceVec;
    cannApiRepo.QueryCompeteSliceByIds(sliceQuery, sliceIds, competeSliceVec);
    EXPECT_EQ(competeSliceVec.size(), 0);
}

/**
 * 测试全量DB的cann下调用栈算子id
 */
TEST_F(CannApiRepoTest, test_QuerySliceIdsByCat_normal)
{
    CannApiRepo cannApiRepo;
    SliceQuery sliceQuery;
    std::vector<uint64_t> sliceIds;
    cannApiRepo.QuerySliceIdsByCat(sliceQuery, sliceIds);
    EXPECT_EQ(sliceIds.size(), 0);
}

/**
 * 测试全量DB的cann下调用栈算子个数
 */
TEST_F(CannApiRepoTest, test_QueryPythonFunctionCountByTrackId_normal)
{
    CannApiRepo cannApiRepo;
    SliceQuery sliceQuery;
    uint64_t count = cannApiRepo.QueryPythonFunctionCountByTrackId(sliceQuery);
    EXPECT_EQ(count, 0);
}

/**
 * 测试全量DB的cann根据trackId查询所有简单算子，无对应track
 */
TEST_F(CannApiRepoTest, test_QuerySimpleSliceWithOutNameByTrackId)
{
    CannApiRepo cannApiRepo;
    SliceQuery sliceQuery;
    std::vector<uint64_t> sliceIds;
    std::vector<SliceDomain> sliceVec;
    cannApiRepo.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
    EXPECT_EQ(sliceVec.size(), 0);
}

/**
 * 测试全量DB的cann根据id集合查询完整算子,无对应track
 */
TEST_F(CannApiRepoTest, test_QueryCompeteSliceByIds_with_track_empty)
{
    CannApiRepo cannApiRepo;
    SliceQuery sliceQuery;
    std::vector<uint64_t> sliceIds = { 1, 2, 3 };
    std::vector<CompeteSliceDomain> competeSliceVec;
    cannApiRepo.QueryCompeteSliceByIds(sliceQuery, sliceIds, competeSliceVec);
    EXPECT_EQ(competeSliceVec.size(), 0);
}

/**
 * 测试全量DB的cann的根据id集合查询完整算子,正常情况
 */
TEST_F(CannApiRepoTest, test_QueryCompeteSliceByIds_with_normal)
{
    TrackInfoManager::Instance().Reset();
    class TableMock : public Dic::Module::Timeline::CannApiTable {
    public:
        void ExcuteQuery(const std::string &fileId, std::vector<CannApiPO> &result) override
        {
            QueryCompeteSliceByIdsWithNormalExcuteQuery(fileId, result);
            ClearThreadLocal();
        }
    };
    std::unique_ptr<Dic::Module::Timeline::CannApiTable> ptr = std::make_unique<TableMock>();
    CannApiRepo cannApiRepo;
    cannApiRepo.SetCannApiTable(std::move(ptr));
    SliceQuery sliceQuery;
    sliceQuery.trackId = TrackInfoManager::Instance().GetTrackId("999", "yyy", "999");
    std::vector<uint64_t> sliceIds = { 1, 2, 3 };
    std::vector<CompeteSliceDomain> competeSliceVec;
    cannApiRepo.QueryCompeteSliceByIds(sliceQuery, sliceIds, competeSliceVec);
    const uint64_t expectSize = 2;
    EXPECT_EQ(competeSliceVec.size(), expectSize);
    auto it = competeSliceVec.begin();
    const uint64_t firstTimestamp = 22;
    EXPECT_EQ(it->timestamp, firstTimestamp);
    it++;
    const uint64_t lastTimestamp = 23;
    EXPECT_EQ(it->timestamp, lastTimestamp);
    TrackInfoManager::Instance().Reset();
}

/**
 * 测试全量DB的cann的根据track查询完整算子,正常情况
 */
TEST_F(CannApiRepoTest, test_QuerySimpleSliceWithOutNameByTrackId_with_normal)
{
    TrackInfoManager::Instance().Reset();
    class TableMock : public Dic::Module::Timeline::CannApiTable {
    public:
        void ExcuteQuery(const std::string &fileId, std::vector<CannApiPO> &result) override
        {
            QuerySimpleSliceWithOutNameByTrackIdWithNormalExcuteQuery(fileId, result);
            ClearThreadLocal();
        }
    };
    std::unique_ptr<Dic::Module::Timeline::CannApiTable> ptr = std::make_unique<TableMock>();
    CannApiRepo cannApiRepo;
    cannApiRepo.SetCannApiTable(std::move(ptr));
    SliceQuery sliceQuery;
    sliceQuery.trackId = TrackInfoManager::Instance().GetTrackId("999", "yyy", "999");
    std::vector<uint64_t> sliceIds = { 1, 2, 3 };
    std::vector<SliceDomain> sliceVec;
    cannApiRepo.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
    const uint64_t expectSize = 2;
    EXPECT_EQ(sliceVec.size(), expectSize);
    auto it = sliceVec.begin();
    const uint64_t firstTimestamp = 22;
    EXPECT_EQ(it->timestamp, firstTimestamp);
    it++;
    const uint64_t lastTimestamp = 23;
    EXPECT_EQ(it->timestamp, lastTimestamp);
    TrackInfoManager::Instance().Reset();
}