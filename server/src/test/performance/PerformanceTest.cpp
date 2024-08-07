/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "PerformanceTest.h"
#include <fstream>

namespace Performance {
CaseExecuteStatus PerformanceTest::status = {};
int PerformanceTest::number = 0;
std::string_view PerformanceTest::test1P2GBRootPath;
std::string_view PerformanceTest::test1P5GBRootPath;

void PerformanceTest::SetUpTestCase()
{
    status = {"No.", "Module", "CaseName", "Result", "Baseline(ms)", "Real Time(ms)"};
    WriteTestCaseResult(status, true);
    number = 0;
#ifdef _WIN32
    test1P2GBRootPath = R"(D:\data\2G\ubuntu125_7611_20240206073731998_ascend_pt)";
    test1P5GBRootPath = R"(D:\data\5G\ubuntu125_72115_20240206115521705_ascend_pt)";
#endif
}

void PerformanceTest::TearDownTestCase() {
}


int PerformanceTest::Main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void PerformanceTest::WriteTestCaseResult(const CaseExecuteStatus& result, bool clear)
{
    std::ofstream outfile;
    outfile.open(TEST_RESULT_FILE_NAME, std::ofstream::out | (clear ? std::ofstream::trunc : std::ofstream::app));

    outfile << result.id << SEPARATOR;
    outfile << result.module << SEPARATOR;
    outfile << result.caseName << SEPARATOR;
    outfile << result.result << SEPARATOR;
    outfile << result.refTime << SEPARATOR;
    outfile << result.realTime << std::endl;
    outfile.close();
}

std::string PerformanceTest::GetNumber()
{
    return std::to_string(++number);
}

void PerformanceTest::SetUp()
{
    Test::SetUp();
    status.id = GetNumber();
    status.module = "";
    status.caseName = "";
    status.result = "failed";
    status.refTime = "";
    status.realTime = "";
}

void PerformanceTest::TearDown()
{
    WriteTestCaseResult(status, false);
    Test::TearDown();
}

}

