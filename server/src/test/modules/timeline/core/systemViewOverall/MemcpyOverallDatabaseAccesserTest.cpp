/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
#include <string>
#include "DataBaseManager.h"
#include "MemcpyOverallDatabaseAccesser.h"
#include "DataEngine.h"
#include "RepositoryFactory.h"
#include "RenderEngine.h"
#include "FullDbParser.h"
#include "ParamsParser.h"
#include "ProjectParserFactory.h"

namespace Dic::Module::Timeline {
class MemcpyOverallDatabaseAccesserIntegrationTest : public ::testing::Test {
public:
    static void InitDataEngine() {
        auto repositoryFactory = RepositoryFactory::Instance();
        auto dataEngine = DataEngine::Instance();
        dataEngine->SetRepositoryFactory(repositoryFactory);
        auto renderEngine = RenderEngine::Instance();
        renderEngine->SetDataEngineInterface(dataEngine);
    }

    static void SetUpTestCase() {
        // 重置全局状态
        FullDb::FullDbParser::Instance().Reset();
        InitDataEngine();

        // 初始化日志系统
        const ParamsOption& option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, std::to_string(option.wsPort));

        // 构建测试数据库路径（增强路径健壮性）
        std::string currPath = Dic::FileUtil::GetCurrPath();
        size_t index = currPath.rfind("server");
        if (index == std::string::npos) {
            FAIL() << "无法定位测试数据库路径：当前路径中不含'server'标识";
        }
        currPath = currPath.substr(0, index - 1);
        testDbPath_ = currPath +
            R"(/test/st/level2/rank_0_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler_0.db)";

        // 配置数据库管理器
        DataBaseManager::Instance().SetDataType(DataType::DB, testDbPath_);
        DataBaseManager::Instance().SetFileType(FileType::PYTORCH, testDbPath_);
        DataBaseManager::Instance().CreateTraceConnectionPool("0", testDbPath_);

        // 执行解析
        std::pair<std::string, ParserType> parserType = {testDbPath_, ParserType::DB};
        auto factory = ParserFactory::GetProjectParser(parserType.second);
        std::string warn;
        std::vector<std::string> parseFileList = factory->GetParseFileByImportFile(testDbPath_, warn);

        Global::ProjectExplorerInfo projectInfo;
        projectInfo.fileName = testDbPath_;
        projectInfo.projectName = testDbPath_;
        projectInfo.projectType = static_cast<int64_t>(factory->GetProjectType({testDbPath_}));
        projectInfo.importType = "import";

        auto parseFileInfo = std::make_shared<ParseFileInfo>();
        parseFileInfo->parseFilePath = testDbPath_;
        projectInfo.subParseFileInfo.push_back(parseFileInfo);

        ImportActionRequest request;
        ImportActionResponse response;
        factory->Parser({projectInfo}, request, response);

        // 设置路径映射
        const std::string projectId = "localhost.localdomain2152938157304401006_0";
        Timeline::DataBaseManager::Instance().SetDbPathMapping("FullDb", testDbPath_, projectId);
        Timeline::DataBaseManager::Instance().SetDbPathMapping("0", testDbPath_, projectId);

        // 安全等待解析完成（带超时机制）
        constexpr int MAX_WAIT_SEC = 120;
        constexpr int CHECK_INTERVAL_MS = 500;
        int elapsedMs = 0;
        while (ParserStatusManager::Instance().GetParserStatus(projectId + " 0") != ParserStatus::FINISH_ALL) {
            if (elapsedMs >= MAX_WAIT_SEC * 1000) {
                FAIL() << "解析超时（超过" << MAX_WAIT_SEC << "秒）: " << projectId;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_INTERVAL_MS));
            elapsedMs += CHECK_INTERVAL_MS;
        }

        // 等待Overlap分析完成
        auto database = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId("0");
        elapsedMs = 0;
        while (!database || !database->CheckValueFromStatusInfoTable(OVERLAP_ANALYSIS_UNIT, FINISH_STATUS)) {
            if (elapsedMs >= MAX_WAIT_SEC * 1000) {
                FAIL() << "Overlap分析超时（超过" << MAX_WAIT_SEC << "秒）";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_INTERVAL_MS));
            elapsedMs += CHECK_INTERVAL_MS;
            database = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId("0");
        }
    }

    static void TearDownTestCase() {
        // 安全清理数据库连接
        auto connList = Timeline::DataBaseManager::Instance().GetAllTraceDatabase();
        for (auto& conn : connList) {
            if (conn) conn->Stop();
        }
        Timeline::DataBaseManager::Instance().Clear();
        Timeline::TraceTime::Instance().Reset();
        Timeline::ParserStatusManager::Instance().ClearAllParserStatus();
    }

protected:
    void SetUp() override {
        // 正确构造被测对象（原构造函数需要database和fileId）
        auto database = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId("0");
        if (!database) {
            FAIL() << "无法获取rank 0的数据库连接";
        }
        // 使用正确的fileId（与SetUpTestCase中创建的连接池一致）
        accessor_ = std::make_unique<MemcpyOverallDatabaseAccesser>(database, testDbPath_);
    }

    void TearDown() override {
        accessor_.reset();
    }

    std::unique_ptr<MemcpyOverallDatabaseAccesser> accessor_;
    static std::string testDbPath_;
};

std::string MemcpyOverallDatabaseAccesserIntegrationTest::testDbPath_ = "";

// ======================
// 测试用例1: 验证基础查询功能（修复参数和字段名）
// ======================
TEST_F(MemcpyOverallDatabaseAccesserIntegrationTest, GetMemcpyRecords_ReturnsValidRecords_WithFullTimeRange) {
    std::vector<MemcpyRecord> records;
    // 使用完整时间范围（UINT64_MAX-1 避免边界问题）
    constexpr uint64_t MAX_TIME = std::numeric_limits<uint64_t>::max() - 1;
    bool success = accessor_->GetMemcpyRecords(0, MAX_TIME, records);

    EXPECT_TRUE(success) << "查询应成功完成";
    EXPECT_GT(records.size(), 0) << "测试数据库应包含Memcpy记录";

    // 验证关键字段（修复字段名：memcpyType 替代 operation）
    if (!records.empty()) {
        for (const auto& rec : records) {
            EXPECT_GT(rec.size, 0u) << "记录大小应大于0";
            EXPECT_FALSE(rec.memcpyType.empty()) << "Memcpy类型不应为空";
            EXPECT_GE(rec.endTime, rec.startTime) << "结束时间应 >= 开始时间";
            EXPECT_DOUBLE_EQ(rec.duration, static_cast<double>(rec.endTime - rec.startTime))
                << "持续时间计算应准确";
            EXPECT_GT(rec.threadId, 0u) << "线程ID应有效";
        }
    }
}

// ======================
// 测试用例2: 验证时间范围过滤功能
// ======================
TEST_F(MemcpyOverallDatabaseAccesserIntegrationTest, GetMemcpyRecords_RespectsTimeRangeFilter) {
    std::vector<MemcpyRecord> allRecords;
    constexpr uint64_t MAX_TIME = std::numeric_limits<uint64_t>::max() - 1;
    accessor_->GetMemcpyRecords(0, MAX_TIME, allRecords);

    if (allRecords.size() < 2) {
        GTEST_SKIP() << "跳过测试：需要至少2条记录验证时间过滤";
    }

    // 获取中间时间点
    uint64_t midTime = (allRecords.front().startTime + allRecords.back().endTime) / 2;

    // 查询前半部分
    std::vector<MemcpyRecord> firstHalf;
    accessor_->GetMemcpyRecords(0, midTime, firstHalf);

    // 查询后半部分
    std::vector<MemcpyRecord> secondHalf;
    accessor_->GetMemcpyRecords(midTime + 1, MAX_TIME, secondHalf);

    // 验证过滤逻辑
    EXPECT_LT(firstHalf.size(), allRecords.size());
    EXPECT_LT(secondHalf.size(), allRecords.size());
    EXPECT_EQ(firstHalf.size() + secondHalf.size(), allRecords.size());

    // 验证时间范围正确性
    for (const auto& rec : firstHalf) {
        EXPECT_LE(rec.endTime, midTime) << "前半部分记录应在时间范围内";
    }
    for (const auto& rec : secondHalf) {
        EXPECT_GT(rec.startTime, midTime) << "后半部分记录应在时间范围内";
    }
}

// ======================
// 测试用例3: 验证空时间范围返回空结果
// ======================
TEST_F(MemcpyOverallDatabaseAccesserIntegrationTest, GetMemcpyRecords_ReturnsEmpty_WhenNoRecordsInTimeRange) {
    std::vector<MemcpyRecord> records;
    // 使用极大时间范围外的区间（应无记录）
    bool success = accessor_->GetMemcpyRecords(
        std::numeric_limits<uint64_t>::max() - 100,
        std::numeric_limits<uint64_t>::max(),
        records
    );

    // 注意：查询成功但无数据是正常行为（返回true）
    EXPECT_TRUE(success) << "空结果集查询应成功";
    EXPECT_TRUE(records.empty()) << "指定时间范围外应无记录";
}

// ======================
// 测试用例4: 验证关键Memcpy操作类型存在
// ======================
TEST_F(MemcpyOverallDatabaseAccesserIntegrationTest, GetMemcpyRecords_ContainsExpectedOperationTypes) {
    std::vector<MemcpyRecord> records;
    constexpr uint64_t MAX_TIME = std::numeric_limits<uint64_t>::max() - 1;
    accessor_->GetMemcpyRecords(0, MAX_TIME, records);

    if (records.empty()) {
        GTEST_SKIP() << "跳过测试：无记录可验证";
    }

    // 统计操作类型
    bool hasH2D = false;
    bool hasD2H = false;

    for (const auto& rec : records) {
        if (rec.memcpyType == "host to device") hasH2D = true;
        if (rec.memcpyType == "device to host") hasD2H = true;
    }

    // 根据PyTorch Profiler典型场景验证
    EXPECT_TRUE(hasH2D || hasD2H) << "应包含主机与设备间数据传输记录";
}

} // namespace Dic::Module::Timeline