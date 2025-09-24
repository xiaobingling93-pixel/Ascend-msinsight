/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "OverlapAnsRepo.h"
#include "TrackInfoManager.h"
#include "DataBaseManager.h"
#include "../../../DatabaseTestCaseMockUtil.cpp"
using namespace Dic::Module::Timeline;
using namespace Dic::Global::PROFILER::MockUtil;

class OverlapAnsRepoTest : public ::testing::Test {
public:
    static std::string g_testDbPath;
    static std::recursive_mutex g_testMutex;
    static Module::Database g_testDataBase;
    const std::string overlap = "CREATE TABLE OVERLAP_ANALYSIS (id INTEGER PRIMARY KEY AUTOINCREMENT, deviceId integer, "
        "startNs integer, endNs integer, type integer);";
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        g_testDbPath = currPath + R"(/src/test/test_data/test_overlap_database.db)";
        g_testDataBase.OpenDb(g_testDbPath, false);
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().CreateTraceConnectionPool("0", g_testDbPath);
    }

    static void TearDownTestSuite()
    {
        g_testDataBase.CloseDb();
        if (FileUtil::CheckFilePathExist(g_testDbPath)) {
            FileUtil::RemoveFile(g_testDbPath);
        }
    }
};
std::string OverlapAnsRepoTest::g_testDbPath;
std::recursive_mutex OverlapAnsRepoTest::g_testMutex;
Module::Database OverlapAnsRepoTest::g_testDataBase(g_testMutex);

TEST_F(OverlapAnsRepoTest, TestOverlayAnsRepoQuerySliceDetailInfoNormal)
{
    g_testDataBase.ExecSql(overlap);
    std::string overlapData =
        "INSERT INTO \"main\".\"OVERLAP_ANALYSIS\" (\"id\", \"deviceId\", \"startNs\", \"endNs\", \"type\") VALUES "
        "(103984, 0, 1742699321190093818, 1742699321190208301, 2);";
    g_testDataBase.ExecSql(overlapData);
    DataBaseManager::Instance().SetDbPathMapping("0", g_testDbPath, "");
    OverlapAnsRepo overlapAnsRepoMock;
    SliceQuery query;
    query.sliceId = "103984";
    query.rankId = "0";
    CompeteSliceDomain slice;
    const bool result = overlapAnsRepoMock.QuerySliceDetailInfo(query, slice);
    EXPECT_EQ(result, true);
}

/**
 * 测试全量DB的 overlapAnsRepo 转化 SliceInterface 的情况
 */
TEST_F(OverlapAnsRepoTest, TestDynamicCastOfMultiSliceInterface)
{
    std::shared_ptr<IBaseSliceRepo> overlapAnsRepo = std::make_shared<OverlapAnsRepo>();
    // 转 IPythonFuncSlice 失败
    const auto pythonFuncRepo = dynamic_cast<IPythonFuncSlice*>(overlapAnsRepo.get());
    EXPECT_EQ(pythonFuncRepo, nullptr);
    // 转 IFindSliceByNameList 失败
    const auto findSliceByNameList = dynamic_cast<IFindSliceByNameList*>(overlapAnsRepo.get());
    EXPECT_EQ(findSliceByNameList, nullptr);
    // 转 IFindSliceByTimepointAndName 失败
    const auto findSliceByTimepointAndName = dynamic_cast<IFindSliceByTimepointAndName*>(overlapAnsRepo.get());
    EXPECT_EQ(findSliceByTimepointAndName, nullptr);
    // 转 ITextSlice 失败
    const auto textSliceRepo = dynamic_cast<ITextSlice*>(overlapAnsRepo.get());
    EXPECT_EQ(textSliceRepo, nullptr);
}
