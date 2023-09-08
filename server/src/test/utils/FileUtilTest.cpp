/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include <gtest/gtest.h>
#include "FileUtil.h"

using namespace Dic;

TEST(SplicePath, BasicAssertions) {
#ifdef _WIN32
    EXPECT_EQ(FileUtil::SplicePath("a", "b"), "a\\b");
#else
    EXPECT_EQ(FileUtil::SplicePath("a", "b"), "a/b");
#endif
}
