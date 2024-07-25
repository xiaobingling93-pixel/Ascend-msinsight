/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "compute/ComputeFuzz.h"

int g_fuzzRunTime = 1000000;
std::string g_reportPath = "./report/";

GTEST_API_ int main(int argc, char **argv)
{
    DT_Set_Report_Path(g_reportPath.c_str());
    DT_SetEnableFork(1);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
