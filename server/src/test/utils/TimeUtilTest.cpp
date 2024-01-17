/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "../TestSuit.cpp"
#include "TimeUtil.h"

using namespace Dic;

int GetCurrentYear()
{
    const int sinceYear = 1900;
    auto currentTime = std::chrono::system_clock::now();
    std::time_t currentTime_t = std::chrono::system_clock::to_time_t(currentTime);
    std::tm *currentTime_tm = std::localtime(&currentTime_t);
    return currentTime_tm->tm_year + sinceYear;
}

int GetCurrentMonth()
{
    const int sinceMonth = 1;
    auto currentTime = std::chrono::system_clock::now();
    std::time_t currentTime_t = std::chrono::system_clock::to_time_t(currentTime);
    std::tm *currentTime_tm = std::localtime(&currentTime_t);
    return currentTime_tm->tm_mon + sinceMonth;
}

int GetCurrentDay()
{
    auto currentTime = std::chrono::system_clock::now();
    std::time_t currentTime_t = std::chrono::system_clock::to_time_t(currentTime);
    std::tm *currentTime_tm = std::localtime(&currentTime_t);
    return currentTime_tm->tm_mday;
}

int GetCurrentHour()
{
    auto currentTime = std::chrono::system_clock::now();
    std::time_t currentTime_t = std::chrono::system_clock::to_time_t(currentTime);
    std::tm *currentTime_tm = std::localtime(&currentTime_t);
    return currentTime_tm->tm_hour;
}

TEST(TimeUtil, Now) {
    const Time now = TimeUtil::Instance().Now();

    int currentYear = GetCurrentYear();
    int currentMonth = GetCurrentMonth();
    int currentDay = GetCurrentDay();
    int currentHour = GetCurrentHour();

    EXPECT_EQ(now.year, currentYear);
    EXPECT_EQ(now.month, currentMonth);
    EXPECT_EQ(now.day, currentDay);
    EXPECT_EQ(now.hour, currentHour);

    EXPECT_TRUE(now.month >= 1 && now.month <= 12);
    EXPECT_TRUE(now.day >= 1 && now.day <= 31);
    EXPECT_TRUE(now.hour >= 0 && now.hour <= 23);
    EXPECT_TRUE(now.minute >= 0 && now.minute <= 59);
    EXPECT_TRUE(now.second >= 0 && now.second <= 59);

    const Time now1 = TimeUtil::Instance().Now();
    const Time now2 = TimeUtil::Instance().Now();
    EXPECT_EQ(now1.year, now2.year);
    EXPECT_EQ(now1.month, now2.month);
    EXPECT_EQ(now1.day, now2.day);
    EXPECT_EQ(now1.hour, now2.hour);
    EXPECT_EQ(now1.minute, now2.minute);
    EXPECT_EQ(now1.second, now2.second);
}

TEST(TimeUtil, NowUTC) {
    uint32_t currentTime = TimeUtil::Instance().NowUTC();

    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    uint32_t expectedTime = static_cast<uint32_t>(now_t);
    EXPECT_GE(currentTime, expectedTime);
}

TEST(TimeUtil, MsToNs) {
    int64_t milliseconds = 100;
    int64_t expectedNanoseconds = 100000000;
    assert(TimeUtil::Instance().MsToNs(milliseconds) == expectedNanoseconds);
}

TEST(TimeUtil, NsToMs) {
    int64_t nanoseconds = 100000000;
    int64_t expectedMilliseconds = 100;
    assert(TimeUtil::Instance().NsToMs(nanoseconds) == expectedMilliseconds);
}

TEST(TimeUtil, NowStr) {
    std::string withMillisec = TimeUtil::Instance().NowStr(TimeStyle::WITH_MILLI_SEC);
    EXPECT_EQ(withMillisec.size(), 23);
    EXPECT_EQ(withMillisec[4], '-');
    EXPECT_EQ(withMillisec[7], '-');
    EXPECT_EQ(withMillisec[10], ' ');
    EXPECT_EQ(withMillisec[13], ':');
    EXPECT_EQ(withMillisec[16], ':');
    EXPECT_EQ(withMillisec[19], '.');

    std::string withoutMillisec = TimeUtil::Instance().NowStr(TimeStyle::WITH_SEC_NO_SPLIT);
    EXPECT_EQ(withoutMillisec.size(), 17);
    EXPECT_EQ(withoutMillisec.find('-'), std::string::npos);
    EXPECT_EQ(withoutMillisec.find(':'), std::string::npos);
    EXPECT_EQ(withoutMillisec.find('.'), std::string::npos);
}