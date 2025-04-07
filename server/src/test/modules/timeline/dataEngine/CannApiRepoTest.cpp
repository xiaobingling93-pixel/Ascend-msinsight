/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "CannApiRepo_mock_data.h"
#include "CannApiRepo.h"
#include "TrackInfoManager.h"
#include "../../../DatabaseTestCaseMockUtil.cpp"
#include "TableDefaultMock.h"
using namespace Dic::TimeLine::CannApiRepo::Mock;
using namespace Dic::TimeLine::Table::Default::Mock;
using namespace Dic::Global::PROFILER::MockUtil;
class CannApiRepoTest : public ::testing::Test {
protected:
    const std::string stringIdsSql = "CREATE TABLE STRING_IDS (id INTEGER PRIMARY KEY,value TEXT);";
    const std::string cannapiSql = "CREATE TABLE CANN_API (startNs INTEGER,endNs INTEGER,type INTEGER,globalTid "
        "INTEGER,connectionId INTEGER PRIMARY KEY,name INTEGER, depth integer);";
    const std::string apiTypeSql = "CREATE TABLE ENUM_API_TYPE (id INTEGER PRIMARY KEY,name TEXT);";
    void SetUp() override
    {
        TrackInfoManager::Instance().Reset();
    }

    void TearDown() override
    {
        TrackInfoManager::Instance().Reset();
    }
};

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

/**
 * 测试根据id查询算子详情,正常情况
 */
TEST_F(CannApiRepoTest, TestQuerySliceDetailInfoNormal)
{
    class CannApiRepoMock : public CannApiRepo {
    public:
        void SetMock(CannDependency &dependency)
        {
            apiTypeTable = std::move(dependency.enumApiTypeTableMock);
            cannApiTable = std::move(dependency.cannApiTableMock);
            stringIdsTable = std::move(dependency.stringIdsTableMock);
        }
    };
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, apiTypeSql);
    DatabaseTestCaseMockUtil::CreateTable(db, cannapiSql);
    DatabaseTestCaseMockUtil::CreateTable(db, stringIdsSql);
    std::string apiInsert = "INSERT INTO \"main\".\"ENUM_API_TYPE\" (\"id\", \"name\") VALUES (10000, 'node');";
    std::string cannInsert = "INSERT INTO \"main\".\"CANN_API\" (\"startNs\", \"endNs\", \"type\", \"globalTid\", "
        "\"connectionId\", \"name\", \"depth\") VALUES (1718180918997508370, 1718180918997541810, "
        "10000, 8785587534250072, 7421, 327, 0);";
    std::string stringInsert = "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (377, 'mmmm');\n"
        "INSERT INTO \"main\".\"STRING_IDS\" (\"id\", \"value\") VALUES (327, 'aaaa');";
    DatabaseTestCaseMockUtil::InsertData(db, apiInsert);
    DatabaseTestCaseMockUtil::InsertData(db, cannInsert);
    DatabaseTestCaseMockUtil::InsertData(db, stringInsert);
    CannDependency dependency;
    dependency.stringIdsTableMock = std::make_unique<StringIdsTableMock>();
    dependency.stringIdsTableMock->SetDb(db);
    dependency.cannApiTableMock = std::make_unique<CannApiTableMock>();
    dependency.cannApiTableMock->SetDb(db);
    dependency.enumApiTypeTableMock = std::make_unique<EnumApiTypeTableMock>();
    dependency.enumApiTypeTableMock->SetDb(db);
    CannApiRepoMock cannApiRepoMock;
    cannApiRepoMock.SetMock(dependency);
    SliceQuery query;
    query.sliceId = "7421";
    query.rankId = "hhh";
    CompeteSliceDomain slice;
    bool result = cannApiRepoMock.QuerySliceDetailInfo(query, slice);
    EXPECT_EQ(result, true);
    EXPECT_EQ(slice.name, "aaaa");
    const uint64_t expectStart = 1718180918997508370;
    const uint64_t expectEnd = 1718180918997541810;
    EXPECT_EQ(slice.timestamp, expectStart);
    EXPECT_EQ(slice.endTime, expectEnd);
    const std::string expectArgs =
        "{\"globalTid\":\"8785587534250072\",\"type\":\"node\",\"name\":\"aaaa\",\"connectionId\":\"7421\"}";
    EXPECT_EQ(slice.args, expectArgs);
}

/**
 * 测试根据id查询算子详情,id不存在的情况
 */
TEST_F(CannApiRepoTest, TestQuerySliceDetailInfoWhenIdNotExistThenReturnFalse)
{
    class CannApiRepoMock : public CannApiRepo {
    public:
        void SetMock(CannDependency &dependency)
        {
            apiTypeTable = std::move(dependency.enumApiTypeTableMock);
            cannApiTable = std::move(dependency.cannApiTableMock);
            stringIdsTable = std::move(dependency.stringIdsTableMock);
        }
    };
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, cannapiSql);
    std::string cannInsert = "INSERT INTO \"main\".\"CANN_API\" (\"startNs\", \"endNs\", \"type\", \"globalTid\", "
        "\"connectionId\", \"name\", \"depth\") VALUES (1718180918997508370, 1718180918997541810, "
        "10000, 8785587534250072, 7421, 327, 0);";
    DatabaseTestCaseMockUtil::InsertData(db, cannInsert);
    CannDependency dependency;
    dependency.cannApiTableMock = std::make_unique<CannApiTableMock>();
    dependency.cannApiTableMock->SetDb(db);
    CannApiRepoMock cannApiRepoMock;
    cannApiRepoMock.SetMock(dependency);
    SliceQuery query;
    query.sliceId = "9999999999\r\f\b\v";
    query.rankId = "hhh";
    CompeteSliceDomain slice;
    bool result = cannApiRepoMock.QuerySliceDetailInfo(query, slice);
    EXPECT_EQ(result, false);
}