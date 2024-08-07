/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "PerformanceTest.h"
#include "DataBaseManager.h"
#include "KernelParse.h"
#include "ParserStatusManager.h"

using namespace Performance;
using namespace Dic;
using namespace Dic::Module;
using namespace Dic::Module::Summary;
const std::string OPERATOR_MODULE = "Operator";
const int TIME_REFERENCE_200MS = 200;
const int TIME_REFERENCE_500MS = 500;

class KernelParseTest : PerformanceTest {
};

TEST_F(PerformanceTest, testKernelParser1P2GBTime)
{
    auto start = std::chrono::high_resolution_clock::now();
    KernelParse::Instance().Parse({std::string(test1P2GBRootPath)});
    while (true) {
        Timeline::ParserStatus status = Timeline::ParserStatusManager::Instance().GetParserStatus(KERNEL_PREFIX + "0");
        if (status == Timeline::ParserStatus::FINISH) {
            break;
        }
        std::chrono::milliseconds duration(10); // 间隔 10 ms
        std::this_thread::sleep_for(duration);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    status.module = OPERATOR_MODULE;
    status.caseName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    status.result = cost <= TIME_REFERENCE_200MS ? "pass" : "failed";
    status.refTime = std::to_string(TIME_REFERENCE_200MS);
    status.realTime = std::to_string(cost);
    KernelParse::Instance().Reset();
}

TEST_F(PerformanceTest, testKernelParser1P5GBTime)
{
    auto start = std::chrono::high_resolution_clock::now();
    KernelParse::Instance().Parse({std::string(test1P5GBRootPath)});
    while (true) {
        Timeline::ParserStatus status = Timeline::ParserStatusManager::Instance().GetParserStatus(KERNEL_PREFIX + "0");
        if (status == Timeline::ParserStatus::FINISH) {
            break;
        }
        std::chrono::milliseconds duration(10); // 间隔 10 ms
        std::this_thread::sleep_for(duration);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    status.module = OPERATOR_MODULE;
    status.caseName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    status.result = cost <= TIME_REFERENCE_500MS ? "pass" : "failed";
    status.refTime = std::to_string(TIME_REFERENCE_500MS);
    status.realTime = std::to_string(cost);
    KernelParse::Instance().Reset();
}