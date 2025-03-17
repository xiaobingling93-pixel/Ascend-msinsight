/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "../TestSuit.cpp"
#include "StringUtil.h"

using namespace Dic;

TEST(StringUtil, IntToString) {
    EXPECT_EQ(StringUtil::IntToString(1, 1), "1");
    EXPECT_EQ(StringUtil::IntToString(1, 2), "01");
    EXPECT_EQ(StringUtil::IntToString(1, 3), "001");
    EXPECT_EQ(StringUtil::IntToString(333, 3), "333");
    EXPECT_EQ(StringUtil::IntToString(100, 1), "100");
    EXPECT_EQ(StringUtil::IntToString(100, 2), "100");
}

TEST(StringUtil, Split) {
    EXPECT_EQ(StringUtil::Split("a,a", ",")[0], "a");
    EXPECT_EQ(StringUtil::Split("a,a", ",")[1], "a");
    EXPECT_EQ(StringUtil::Split("wwa,aww", "wa,aw")[0], "w");
    EXPECT_EQ(StringUtil::Split("wwa,aww", "wa,aw")[1], "w");
    EXPECT_EQ(StringUtil::Split("wwa,aww", "a.a")[0], "ww");
    EXPECT_EQ(StringUtil::Split("wwa,aww", "aww")[0], "wwa,");
}

TEST(StringUtil, ByteNum) {
    std::cout << StringUtil::ByteNum(11) << std::endl;
}

TEST(StringUtil, IsAllDigitsWithNormalInput) {
    EXPECT_EQ(StringUtil::IsAllDigits("123"), true);
    EXPECT_EQ(StringUtil::IsAllDigits("1"), true);
    EXPECT_EQ(StringUtil::IsAllDigits("1+2"), false);
    EXPECT_EQ(StringUtil::IsAllDigits("-2"), false);
}

TEST(StringUtil, AnonymousString) {
    EXPECT_EQ(StringUtil::AnonymousString("a"), "a");
    EXPECT_EQ(StringUtil::AnonymousString("ab"), "ab");
    EXPECT_EQ(StringUtil::AnonymousString("abc"), "a*c");
    EXPECT_EQ(StringUtil::AnonymousString("abcd"), "a*cd");
    EXPECT_EQ(StringUtil::AnonymousString("abcd1111aaaaaa"), "abcd****aaaaaa");
    EXPECT_EQ(StringUtil::AnonymousString("1111awfdcssf"), "1111****cssf");
    EXPECT_EQ(StringUtil::AnonymousString("************"), "************");
}

TEST(StringUtil, Trim) {
    std::string str1 = " Hello,World! ";
    EXPECT_EQ(StringUtil::Trim(str1), "Hello,World!");
    std::string str2 = "Hello,World! ";
    EXPECT_EQ(StringUtil::Trim(str2), "Hello,World!");
    std::string str3 = "";
    EXPECT_EQ(StringUtil::Trim(str3), "");
    std::string str4 = "  ";
    EXPECT_EQ(StringUtil::Trim(str4), "");
}


#ifdef _WIN32
TEST(StringUtil, GbkToUtf8) {
    EXPECT_EQ(StringUtil::GbkToUtf8(nullptr), "");
    EXPECT_EQ(StringUtil::GbkToUtf8("nullptr"), "nullptr");
    EXPECT_EQ(StringUtil::GbkToUtf8("中国"), "\xE6\xB6\x93\xEE\x85\x9E\xE6\xB5\x97");
    EXPECT_EQ(StringUtil::GbkToUtf8("中"), "\xE6\xB6\x93?");
}

TEST(StringUtil, Utf8ToGbk) {
    EXPECT_EQ(StringUtil::Utf8ToGbk(nullptr), "");
    EXPECT_EQ(StringUtil::Utf8ToGbk("nullptr"), "nullptr");
    EXPECT_EQ(StringUtil::Utf8ToGbk("中国"), "\xD6\xD0\xB9\xFA");
    EXPECT_EQ(StringUtil::Utf8ToGbk("中"), "\xD6\xD0");
}

#endif


TEST(StringUtil, join) {
    std::vector<std::string> list1 = {"a", "b", "c"};
    EXPECT_EQ(StringUtil::join(list1, "1"), "a1b1c");
    std::vector<std::string> list2 = {"a", "b", "c"};
    EXPECT_EQ(StringUtil::join(list2, ","), "a,b,c");
}

TEST(StringUtil, StartWith) {
    EXPECT_EQ(StringUtil::StartWith("aaaa", "aa"), true);
    EXPECT_EQ(StringUtil::StartWith("xdw", "wa"), false);
    EXPECT_EQ(StringUtil::StartWith("xa", " "), false);
    EXPECT_EQ(StringUtil::StartWith(" xd", " "), true);
}

TEST(StringUtil, EndWith) {
    EXPECT_EQ(StringUtil::EndWith("aaaa", "aa"), true);
    EXPECT_EQ(StringUtil::EndWith("xdw", "w"), true);
    EXPECT_EQ(StringUtil::EndWith("xa", " "), false);
    EXPECT_EQ(StringUtil::EndWith(" xd ", " "), true);
}

TEST(StringUtil, Contains) {
    EXPECT_EQ(StringUtil::Contains(" xdw", "x"), true);
    EXPECT_EQ(StringUtil::Contains(" xdw", " "), true);
    EXPECT_EQ(StringUtil::Contains("xdw", "d"), true);
    EXPECT_EQ(StringUtil::Contains("xdw", "ad"), false);
}

TEST(StringUtil, ReplaceFirstWithNormalInput) {
    EXPECT_EQ(StringUtil::ReplaceFirst("xdw", "x", "y"), "ydw");
    EXPECT_EQ(StringUtil::ReplaceFirst(" xdw", " ", ""), "xdw");
    EXPECT_EQ(StringUtil::ReplaceFirst(" xdw ", " ", ""), "xdw ");
    EXPECT_EQ(StringUtil::ReplaceFirst("aaa ", "b", "c"), "aaa ");
}

TEST(StringUtil, ValidateCommandFilePathParamWithNormalInput) {
#ifdef _WIN32
    EXPECT_EQ(StringUtil::ValidateCommandFilePathParam(""), false);
#else
    EXPECT_EQ(StringUtil::ValidateCommandFilePathParam(""), false);
    EXPECT_EQ(StringUtil::ValidateCommandFilePathParam("/"), true);
    EXPECT_EQ(StringUtil::ValidateCommandFilePathParam("/home/xxx/a|b"), false);
    EXPECT_EQ(StringUtil::ValidateCommandFilePathParam("/home/xxx/${a}"), false);
    EXPECT_EQ(StringUtil::ValidateCommandFilePathParam("/home/xxx/cc;mkdir d"), false);
#endif
}

TEST(StringUtil, ToCamelCaseWithNormalInput) {
    EXPECT_EQ(StringUtil::ToCamelCase("rank_id"), "rankId");
    EXPECT_EQ(StringUtil::ToCamelCase("rank_id, device_Id"), "rankId, deviceId");
    EXPECT_EQ(StringUtil::ToCamelCase("rank__id, device_Id"), "rankId, deviceId");
}

TEST(StringUtil, SplitStringWithParenthesesByCommaTestReturnEmptyWhenEmptyInput)
{
    EXPECT_EQ(StringUtil::SplitStringWithParenthesesByComma("").size(), 0);
}

TEST(StringUtil, SplitStringWithParenthesesByCommaTestReturnNormalWhenInputWithSpace)
{
    auto result = StringUtil::SplitStringWithParenthesesByComma("( a ,b,  c d, )");
    EXPECT_EQ(result.size(), 3); // 3, a, b, c
    EXPECT_EQ(result.at(result.size() - 1), "c d");
}

TEST(StringUtil, CreateQuestionMarkStringWhenInputZero)
{
    auto result = StringUtil::CreateQuestionMarkString(0);
    EXPECT_EQ(result, "");
}

TEST(StringUtil, CreateQuestionMarkStringWhenInputNotZero)
{
    auto result = StringUtil::CreateQuestionMarkString(5);
    EXPECT_EQ(result, "?,?,?,?,?");
}