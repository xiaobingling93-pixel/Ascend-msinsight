/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_PERFORMANCETEST_H
#define PROFILER_SERVER_PERFORMANCETEST_H

#include <string>
#include <gtest/gtest.h>

namespace Performance {
struct CaseExecuteStatus {
    std::string id;
    std::string module;
    std::string caseName;
    std::string result;
    std::string refTime;
    std::string realTime;
};
const std::string SEPARATOR = ",";
const std::string TEST_RESULT_FILE_NAME = "performance_test.csv";

class PerformanceTest : public ::testing::Test {
public:
    static std::string GetNumber();

    static void WriteTestCaseResult(const CaseExecuteStatus& result, bool clear);

protected:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp() override;

    void TearDown() override;

    static int Main(int argc, char** argv);

    static int number;
    static CaseExecuteStatus status;
    static std::string_view test1P2GBRootPath;
    static std::string_view test1P5GBRootPath;
};

}

#endif // PROFILER_SERVER_PERFORMANCETEST_H
