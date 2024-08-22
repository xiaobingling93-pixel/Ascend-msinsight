/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_NUMBER_UTIL_H
#define DATA_INSIGHT_CORE_NUMBER_UTIL_H

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "cmath"

namespace Dic {
const int INVALID_NUMBER = 0xffffffff;
// 16进制
const int HEXADECIMAL = 16;
const std::string THOUSAND_SUFFIX = "000";
const int THOUSAND = 1000;
const int64_t MAX = pow(10, 12);
class NumberUtil {
public:
    static inline int TryParseInt(const std::string &intStr)
    {
        int ret;
        try {
            ret = std::stoi(intStr);
        } catch (std::invalid_argument &) {
            // no conversion
            return INVALID_NUMBER;
        } catch (std::out_of_range &) {
            // out of range
            return INVALID_NUMBER;
        }
        return ret;
    }

    static inline unsigned long long TryParseUnsignedLongLong(const std::string &longLongStr)
    {
        int ret;
        try {
            ret = std::stoull(longLongStr);
        } catch (std::invalid_argument &) {
            // no conversion
            return INVALID_NUMBER;
        } catch (std::out_of_range &) {
            // out of range
            return INVALID_NUMBER;
        }
        return ret;
    }

    static inline std::string Uint64ToHexString(uint64_t number)
    {
        std::ostringstream ss;
        ss << "0x" << std::hex << number;
        return ss.str();
    }

    static inline int HexadecimalStrToDecimalInt(const std::string &hexadecimalStr)
    {
        try {
            int size = std::stoi(hexadecimalStr, nullptr, HEXADECIMAL);
            return size;
        } catch (std::invalid_argument &) {
            // no conversion
            return INVALID_NUMBER;
        } catch (std::out_of_range &) {
            // out of range
            return INVALID_NUMBER;
        }
    }

    static inline int64_t TimestampUsToNs(long double us)
    {
        // 当数字非常大时使用乘法，部分数字会损失精度，故改为字符串操作
        if (us < MAX) {
            return llroundl(us * THOUSAND);
        }
        return TimestampUsToNs(std::to_string(us));
    }

    static inline int64_t TimestampUsToNs(std::string us)
    {
        auto str = us.append(THOUSAND_SUFFIX);
        auto index = str.find('.');
        if (index != std::string::npos) {
            str.replace(index, 1, "");
            str.insert(index + THOUSAND_SUFFIX.length(), ".");
        }
        try {
            return llroundl(std::stold(str));
        } catch (std::exception &) {
            return 0;
        }
    }

    static inline double StringToDouble(const std::string& usStr)
    {
        if (usStr.empty()) {
            return 0;
        }
        try {
            return std::stod(usStr);
        } catch (std::exception &) {
            return 0;
        }
    }

    static inline long double StringToLongDouble(const std::string& usStr)
    {
        if (usStr.empty()) {
            return 0;
        }
        try {
            return std::stold(usStr);
        } catch (std::exception &) {
            return 0;
        }
    }

    static inline long StringToLong(const std::string& usStr)
    {
        if (usStr.empty()) {
            return 0;
        }
        try {
            return std::stol(usStr);
        } catch (std::exception &) {
            return 0;
        }
    }

    static inline std::string StringDoubleMinus(const std::string &str1, const std::string &str2, int precision = 3)
    {
        long double num1 = StringToLongDouble(str1);
        long double num2 = StringToLongDouble(str2);
        long double difference = num1 - num2;
        std::stringstream sstream;
        sstream << std::fixed << std::setprecision(precision) << difference;
        return sstream.str();
    }

    // 只处理1~6位小数位的截尾
    static inline double DoubleReservedNDigits(double data, int n)
    {
        if (n <= 0 || n > 6) { // 最多处理6位小数位
            return data;
        }

        int ratio = 1;
        for (int i = 0; i < n; ++i) {
            ratio *= 10; // 10进制
        }
        long double temp = data;
        return (double)(std::round(data * ratio) / ratio);
    }

    static inline bool IsGreater(float a, float b, float epsilon = 1e-9)
    {
        if (std::fabs(a - b) < epsilon) { // 如何两个浮点数只差小于epsilon, 则认为两数相等
            return false;
        } else {
            return a > b;
        }
    }
};
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_NUMBER_UTIL_H
