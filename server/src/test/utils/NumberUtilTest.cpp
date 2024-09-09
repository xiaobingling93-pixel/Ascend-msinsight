/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "../TestSuit.cpp"
#include "NumberUtil.h"

using namespace Dic;

TEST(NumberUtil, TryParseInt) {
    EXPECT_EQ(NumberUtil::TryParseInt("1"), 1);
    EXPECT_EQ(NumberUtil::TryParseInt("129"), 129);
    EXPECT_EQ(NumberUtil::TryParseInt("1024"), 1024);
    EXPECT_EQ(NumberUtil::TryParseInt("A"), INVALID_NUMBER);
    EXPECT_EQ(NumberUtil::TryParseInt("99999999999999999999"), INVALID_NUMBER);
}

TEST(NumberUtil, Uint64ToHexString) {
    EXPECT_EQ(NumberUtil::Uint64ToHexString(1), "0x1");
    EXPECT_EQ(NumberUtil::Uint64ToHexString(2), "0x2");
    EXPECT_EQ(NumberUtil::Uint64ToHexString(10), "0xa");
    EXPECT_EQ(NumberUtil::Uint64ToHexString(11), "0xb");
    EXPECT_EQ(NumberUtil::Uint64ToHexString(999999999), "0x3b9ac9ff");
    EXPECT_EQ(NumberUtil::Uint64ToHexString(99999999999999), "0x5af3107a3fff");
}

TEST(NumberUtil, HexadecimalStrToDecimalInt) {
    EXPECT_EQ(NumberUtil::HexadecimalStrToDecimalInt("0x1"), 1);
    EXPECT_EQ(NumberUtil::HexadecimalStrToDecimalInt("0x2"), 2);
    EXPECT_EQ(NumberUtil::HexadecimalStrToDecimalInt("0xa"), 10);
    EXPECT_EQ(NumberUtil::HexadecimalStrToDecimalInt("0xb"), 11);
    EXPECT_EQ(NumberUtil::HexadecimalStrToDecimalInt("0x3b9ac9ff"), 999999999);
}

TEST(NumberUtil, TimestampUsToNs) {
    double a = stod("1695297849996490.053");
    EXPECT_EQ(1695297849996489984, llround(a * 1000));
    EXPECT_EQ(1695297849996490053, NumberUtil::TimestampUsToNs(stold("1695297849996490.053011100000")));
    EXPECT_EQ(1695297849996490000, NumberUtil::TimestampUsToNs(1695297849996490));
}

TEST(NumberUtil, DoubleReservedNDigits) {
    double a = stod("490.053849996");
    EXPECT_EQ(NumberUtil::DoubleReservedNDigits(a, 0), a);
    EXPECT_EQ(NumberUtil::DoubleReservedNDigits(a, 7), a);
    EXPECT_EQ(NumberUtil::DoubleReservedNDigits(a, 5), stod("490.05385"));
}

TEST(NumberUtil, StringToDouble) {
    EXPECT_EQ(NumberUtil::StringToDouble(""), 0);
    EXPECT_EQ(NumberUtil::StringToDouble("xxx"), 0);
    EXPECT_EQ(NumberUtil::StringToDouble("490.053"), 490.053); // 490.053
}

TEST(NumberUtil, StringToLongDouble) {
    EXPECT_EQ(NumberUtil::StringToLongDouble(""), 0);
    EXPECT_EQ(NumberUtil::StringToLongDouble("xxx"), 0);
    EXPECT_EQ(NumberUtil::StringToLongDouble("490.053"), std::stold("490.053")); // 490.053
}

TEST(NumberUtil, StringToLong) {
    EXPECT_EQ(NumberUtil::StringToLong(""), 0);
    EXPECT_EQ(NumberUtil::StringToLong("xxx"), 0);
    EXPECT_EQ(NumberUtil::StringToLong("490"), 490); // 490
}

TEST(NumberUtil, StringRservedNDigits)
{
    EXPECT_EQ(NumberUtil::StrReservedNDigits("3.1415", 2), "3.14");
    EXPECT_EQ(NumberUtil::StrReservedNDigits("3.1", 2), "3.1");
    EXPECT_EQ(NumberUtil::StrReservedNDigits("31", 2), "31");
    EXPECT_EQ(NumberUtil::StrReservedNDigits("0.0002", 2), "0.0002");
    EXPECT_EQ(NumberUtil::StrReservedNDigits("0.0002", 3), "0.0002");
}