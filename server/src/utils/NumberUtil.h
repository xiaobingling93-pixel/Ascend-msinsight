/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_NUMBER_UTIL_H
#define DATA_INSIGHT_CORE_NUMBER_UTIL_H

#include <string>
#include <iostream>
namespace Dic {
const int INVALID_NUMBER = 0xffffffff;
// 16进制
const int HEXADECIMAL = 16;
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

    static inline bool IsNumber(const std::string& str)
    {
        for (char i : str) {
            if (!isdigit(i)) {
                return false;
            }
        }
        return true;
    }

    static inline bool RankIdCompare(const std::string &a, const std::string &b)
    {
        if (NumberUtil::IsNumber(a) && NumberUtil::IsNumber(b)) {
            return std::stoi(a) < std::stoi(b);
        }
        return a < b;
    }
};
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_NUMBER_UTIL_H
