/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
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
#include <ctime>
#include <string>
#include "gtest/gtest.h"
#include "utils/CmdUtil.h"
#include "utils/ExecUtil.h"

using namespace Dic;

class CmdExecUtilTest : public testing::Test {
};

TEST_F(CmdExecUtilTest, CommandLs)
{
    CmdUtil executor = CmdUtil("/bin/date");
    EXPECT_EQ(executor.Valid(), true);
    executor.Args("+%Y");
    std::string output;
    bool success = executor.ExecuteWithResult(output);
    EXPECT_EQ(success, true);
    time_t now;
    time(&now);
    // 转换为本地时间
    tm *local = localtime(&now);
    // 获取年份
    constexpr int yearStart = 1900;
    constexpr int base = 10;
    int year = local->tm_year + yearStart;  // tm_year存储的是从1900年开始的年份
    EXPECT_EQ(std::stoi(output, nullptr, base), year);
}

TEST_F(CmdExecUtilTest, InvalidCmd)
{
    // not exist
    CmdUtil executor = CmdUtil("/bin/data;");
    EXPECT_EQ(executor.Valid(), false);
    std::string output;
    EXPECT_EQ(executor.ExecuteWithResult(output), false);
    // empty
    CmdUtil executor2 = CmdUtil("");
    EXPECT_EQ(executor2.Valid(), false);
    // no exec permission
    CmdUtil executor3 = CmdUtil("/etc/hosts");
    EXPECT_EQ(executor2.Valid(), false);
}

TEST_F(CmdExecUtilTest, InvalidArgs)
{
    CmdUtil executor = CmdUtil("/bin/date");
    executor.Args("");
    executor.Args("+%s");
    executor.Args(";ji");
    std::string output;
    bool success = executor.ExecuteWithResult(output);
    EXPECT_EQ(success, true);
    time_t currentTime;
    time(&currentTime);
    EXPECT_EQ(currentTime, std::stol(output));
}

TEST_F(CmdExecUtilTest, ExecUtil)
{
    ExecUtil execUtil;
    std::string result = execUtil.Exec("/bin/date +%s");
    time_t currentTime;
    time(&currentTime);
    EXPECT_EQ(currentTime, std::stol(result));
}
