/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_NUMBER_UTIL_H
#define DATA_INSIGHT_CORE_NUMBER_UTIL_H

#include <string>
#include <iostream>
#include <sstream>
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
};
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_NUMBER_UTIL_H
