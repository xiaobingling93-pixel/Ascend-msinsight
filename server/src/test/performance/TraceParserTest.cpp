/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "PerformanceTest.h"
#include "FileUtil.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"

using namespace Performance;
using namespace Dic;
using namespace Dic::Module;
using namespace Dic::Module::Timeline;
const std::string TIMELINE_MODULE = "Timeline";
const int TIME_REFERENCE_1S = 1000;
const int TIME_REFERENCE_2S = 2000;

class TraceParserTest : PerformanceTest {
};

TEST_F(PerformanceTest, testTraceParser1P2GBTime)
{
    auto start = std::chrono::high_resolution_clock::now();
    DataBaseManager::Instance().SetDataType(DataType::TEXT, <#initializer#>);
    DataBaseManager::Instance().CreateTraceConnectionPool("0",
        std::string(test1P2GBRootPath) + R"(/ASCEND_PROFILER_OUTPUT/mindstudio_insight_data.db)");
    TraceFileParser::Instance().Parse(
        {std::string(test1P2GBRootPath) + R"(/ASCEND_PROFILER_OUTPUT/trace_view.json)"},
        "0", "", "");
    while (true) {
        ParserStatus status = ParserStatusManager::Instance().GetParserStatus("0");
        if (status == ParserStatus::FINISH) {
            break;
        }
        std::chrono::milliseconds duration(50); // 间隔 50 ms
        std::this_thread::sleep_for(duration);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    status.module = TIMELINE_MODULE;
    status.caseName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    status.result = cost <= TIME_REFERENCE_1S ? "pass" : "failed";
    status.refTime = std::to_string(TIME_REFERENCE_1S);
    status.realTime = std::to_string(cost);
    TraceFileParser::Instance().Reset();
}

TEST_F(PerformanceTest, testTraceParser1P5GBTime)
{
    auto start = std::chrono::high_resolution_clock::now();
    DataBaseManager::Instance().SetDataType(DataType::TEXT, <#initializer#>);
    DataBaseManager::Instance().CreateTraceConnectionPool("0",
        std::string(test1P5GBRootPath) + R"(/ASCEND_PROFILER_OUTPUT/mindstudio_insight_data.db)");
    TraceFileParser::Instance().Parse(
        {std::string(test1P5GBRootPath) + R"(/ASCEND_PROFILER_OUTPUT/trace_view.json)"},
        "0", "", "");
    while (true) {
        ParserStatus status = ParserStatusManager::Instance().GetParserStatus("0");
        if (status == ParserStatus::FINISH) {
            break;
        }
        std::chrono::milliseconds duration(50); // 间隔 50 ms
        std::this_thread::sleep_for(duration);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    status.module = TIMELINE_MODULE;
    status.caseName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    status.result = cost <= TIME_REFERENCE_2S ? "pass" : "failed";
    status.refTime = std::to_string(TIME_REFERENCE_2S);
    status.realTime = std::to_string(cost);
    TraceFileParser::Instance().Reset();
}