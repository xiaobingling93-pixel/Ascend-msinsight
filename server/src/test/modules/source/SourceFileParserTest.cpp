/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SourceFileParser.h"
#include "SourceProtocolRequest.h"
#include "ProjectParserFactory.h"
#include "../../TestSuit.cpp"

using namespace std;
using namespace Dic::Module::Source;

class SourceFileParserTest : public ::testing::Test {
public:
    static std::string dataPath;
    static std::string dbPath;

    static void SetUpTestCase()
    {
        string currPath = Dic::FileUtil::GetCurrPath();
        auto index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        dataPath = currPath + R"(/src/test/test_data/data.bin)";
        dbPath = currPath + R"(/src/test/test_data/compute_mindstudio_insight_data.db)";
        DataBaseManager::Instance().SetDataType(DataType::TEXT);
        DataBaseManager::Instance().CreatConnectionPool(dataPath, dbPath);
    }

    static void TearDownTestCase()
    {
        SourceFileParser::Instance().Reset();
        DataBaseManager::Instance().Clear();
        DataBaseManager::Instance().ReleaseDatabaseByRankId(dbPath);
        if (std::remove(dbPath.c_str()) == 0) {
            ServerLog::Info("Remove database file success.");
        } else {
            ServerLog::Info("Remove database file failed: ", dbPath);
        }
    }
};

std::string SourceFileParserTest::dataPath;
std::string SourceFileParserTest::dbPath;

static void WaitParseEnd(std::vector<std::string>&& statusList)
{
    if (statusList.empty()) {
        return;
    }
    while (true) {
        size_t i = 0;
        for (const auto& tmp : statusList) {
            if (ParserStatusManager::Instance().GetParserStatus(tmp) != ParserStatus::FINISH) {
                break;
            } else {
                i++;
            }
        }
        if (i < statusList.size()) {
            continue;
        } else {
            Dic::Server::ServerLog::Info("parse end");
            return;
        }
    }
}

TEST_F(SourceFileParserTest, Parse)
{
    auto& parser = SourceFileParser::Instance();
    parser.SetFilePath(dataPath);
    parser.Parse(std::vector<std::string>(), dataPath, dataPath, dbPath);
    // 等待解析任务完成
    WaitParseEnd({dataPath});
    auto list = parser.GetSourceList();
    int sourceListSize = 6;
    EXPECT_EQ(list.size(), sourceListSize);
    parser.Reset();
}