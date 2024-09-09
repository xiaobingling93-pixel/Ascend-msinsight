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
constexpr int MAX_RESERVED_DIGITS = 6;
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

    /**
     * 去除浮点数字符串后缀0
     * @param numStr 浮点数字符串
     * @return 去除后缀0的结果
     */
    static inline std::string RemoveTrailingZerosAndDecimal(const std::string &numStr)
    {
        // 找到最后一个非零字符的位置
        size_t pos = numStr.find_last_not_of('0');
        // 如末尾没有零字符，直接返回
        if (pos == numStr.length() - 1) {
            return numStr;
        }
        // 如果所有字符都是零，则返回0
        if (pos == std::string::npos) {
            return "0";
        }
        // 检查小数点后的零是否都是零
        if (numStr[pos] == '.') {
            return numStr.substr(0, pos);
        }
        // 返回去除末尾0的结果
        return numStr.substr(0, pos + 1);
    }

    /**
     * 字符串浮点数减法（去除后缀0）
     * @param str1 被减数
     * @param str2 减数
     * @param maxPrecision 最大保留位数
     * @return 减法结果
     */
    static inline std::string StringDoubleMinusWithoutTrailingZero(const std::string &str1, const std::string &str2,
                                                                   int maxPrecision = 3)
    {
        if (str1.empty() || str2.empty()) {
            return "";
        }
        std::string minusRes =  StringDoubleMinus(str1, str2, maxPrecision);
        return RemoveTrailingZerosAndDecimal(minusRes);
    }

    // 只处理1~6位小数位的截尾
    static inline double DoubleReservedNDigits(double data, int n = 6)
    {
        if (n <= 0 || n > MAX_RESERVED_DIGITS) { // 最多处理6位小数位
            return data;
        }

        int ratio = 1;
        for (int i = 0; i < n; ++i) {
            ratio *= 10; // 10进制
        }
        long double temp = data;
        return (double)(std::round(data * ratio) / ratio);
    }

    static inline double Sub(double a, double b)
    {
        return DoubleReservedNDigits(DoubleReservedNDigits(a) - DoubleReservedNDigits(b));
    }

    static inline std::string StrReservedNDigits(const std::string& data, int n)
    {
        if (n <= 0 || n > MAX_RESERVED_DIGITS) {
            return data;
        }
        auto pos  = data.find_last_of('.');
        if (pos != std::string::npos) {
            std::string sub =  data.substr(0, pos + n + 1);
            // 保留小数后为零
            bool isZero = std::all_of(sub.begin(), sub.end(), [](char c) { return (c == '0' || c == '.'); });
            return isZero ? data : sub;
        }
        return data;
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
