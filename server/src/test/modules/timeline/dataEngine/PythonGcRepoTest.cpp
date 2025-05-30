/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "PythonGcRepo.h"
#include "TrackInfoManager.h"
#include "../../../DatabaseTestCaseMockUtil.cpp"
#include "TableDefaultMock.h"
using namespace Dic::Module::Timeline;
using namespace Dic::TimeLine::Table::Default::Mock;
using namespace Dic::Global::PROFILER::MockUtil;
class PythonGCRepoTest : public ::testing::Test {
protected:
    const std::string gcSql = "CREATE TABLE GC_RECORD (startNs INTEGER,endNs INTEGER, globalTid INTEGER);";
    void SetUp() override
    {
        TrackInfoManager::Instance().Reset();
    }

    void TearDown() override
    {
        TrackInfoManager::Instance().Reset();
    }
};

TEST_F(PythonGCRepoTest, TestQuerySliceDetailInfoNormal)
{
    class PythonGCRepoMock : public PythonGcRepo {
    public:
        void SetMock(PythonGcDependency &dependency)
        {
            table = std::move(dependency.tableMock);
        }
    };
    sqlite3 *db = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(db);
    DatabaseTestCaseMockUtil::CreateTable(db, gcSql);
    std::string pythonGcInsert = "INSERT INTO (\"startNs\", \"endNs\", \"globalTid\") VALUES "
        "(1718180918997410110, 1718180918997410110, 65535);";
    DatabaseTestCaseMockUtil::InsertData(db, pythonGcInsert);

    PythonGcDependency dependency;
    dependency.tableMock = std::make_unique<PythonGCTableMock>();
    dependency.tableMock->SetDb(db);
    PythonGCRepoMock pythonGCRepoMock;
    pythonGCRepoMock.SetMock(dependency);
    SliceQuery query;
    query.sliceId = "1";
    query.rankId = "hhh";
    CompeteSliceDomain slice;
    bool result = pythonGCRepoMock.QuerySliceDetailInfo(query, slice);
    EXPECT_EQ(result, false);
}
