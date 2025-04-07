/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SafeFile.h"

class SafeFileTest : public ::testing::Test {
};

TEST_F(SafeFileTest, testOpenReadFileSafelyWhenFileCanNotBeWrittenByOthers)
{
    std::string path = "FileCanNotBeWrittenByOthers.tmp";
    std::string content = "test";
    std::ofstream out(path);
    out.write(content.c_str(), content.size());
    out.close();
    // 设置仅owner拥有写权限
    fs::permissions(path, fs::perms::owner_all);

    // 读取文件并比较内容
    auto in = Dic::OpenReadFileSafely(path, std::ios::in);
    EXPECT_TRUE(in);
    std::string readLine;
    readLine.resize(content.length());
    in.read(&readLine[0], content.length());
    in.close();
    EXPECT_EQ(content, readLine);

    // 删除文件
    std::remove(path.c_str());
}