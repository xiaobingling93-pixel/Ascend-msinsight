/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: String Utility declaration
 */

#ifndef DATA_INSIGHT_CORE_STRING_UTIL_H
#define DATA_INSIGHT_CORE_STRING_UTIL_H

#include <string>
#include <regex>
#include <cstdint>
#include <memory>
#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <map>

namespace Dic {
class StringUtil {
public:
    static inline const std::string IntToString(int number, int length)
    {
        std::string s = std::to_string(number);
        while (s.length() < length) {
            s = '0' + s;
        }
        return s;
    }

    static inline const std::vector<std::string> Split(const std::string &str, const std::string &regexStr)
    {
        std::regex regexz(regexStr);
        std::vector<std::string> list(std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
            std::sregex_token_iterator());
        return list;
    }

    static std::uint8_t ByteNum(const std::uint8_t &byte)
    {
        const static std::map<std::uint8_t, std::uint8_t> MAP_BYTES = { { 0xE0, 0xC0 },
                                                                        { 0xF0, 0xE0 },
                                                                        { 0xF8, 0xF0 },
                                                                        { 0xFC, 0xF8 },
                                                                        { 0xFE, 0xFC } };
        uint8_t index = 0;
        uint8_t byteNum = 0;
        for (auto kv : MAP_BYTES) {
            index++;
            if ((byte & kv.first) == kv.second) {
                byteNum = index;
                break;
            }
        }
        return byteNum;
    }

    static bool IsUtf8String(const std::string &str)
    {
        std::uint32_t byteNum = 0;
        std::uint8_t byte;
        for (int i = 0; str[i] != '\0'; i++) {
            byte = static_cast<std::uint8_t>(str[i]);
            // ASCII
            if ((byteNum == 0) && ((byte & 0x80) == 0)) {
                continue;
            }
            // UTF8 first byte
            if (byteNum == 0) {
                byteNum = ByteNum(byte);
                if (byteNum == 0) {
                    return false;
                }
            } else { // UTF8 other bytes
                if ((byte & 0xC0) != 0x80) {
                    return false;
                }
                byteNum--;
            }
        }
        return true;
    }

    static inline std::string AnonymousString(const std::string &str)
    {
        const uint16_t MIN_ANONYMOUS_LEN = 3;
        if (str.length() < MIN_ANONYMOUS_LEN) {
            return str;
        }
        std::string res(str);
        int pos = res.length() / MIN_ANONYMOUS_LEN;
        res.replace(pos, pos, pos, '*');
        return res;
    }

    static inline std::string Trim(std::string &str)
    {
        if (str.empty()) {
            return str;
        }
        str.erase(0, str.find_first_not_of(" "));
        str.erase(str.find_last_not_of(" ") + 1);
        return str;
    }

#ifdef _WIN32
    static std::string GbkToUtf8(const char *srcStr)
    {
        if (srcStr == nullptr) {
            return "";
        }
        int len = MultiByteToWideChar(CP_ACP, 0, srcStr, -1, nullptr, 0);
        std::unique_ptr<wchar_t[]> wstr = std::make_unique<wchar_t[]>(len + 1);
        MultiByteToWideChar(CP_ACP, 0, srcStr, -1, wstr.get(), len);
        len = WideCharToMultiByte(CP_UTF8, 0, wstr.get(), -1, nullptr, 0, nullptr, nullptr);
        std::unique_ptr<char[]> str = std::make_unique<char[]>(len + 1);
        WideCharToMultiByte(CP_UTF8, 0, wstr.get(), -1, str.get(), len, nullptr, nullptr);
        return std::string(str.get());
    }
    static std::string Utf8ToGbk(const char *srcStr)
    {
        if (srcStr == nullptr) {
            return "";
        }
        int len = MultiByteToWideChar(CP_UTF8, 0, srcStr, -1, nullptr, 0);
        std::unique_ptr<wchar_t[]> wstr = std::make_unique<wchar_t[]>(len + 1);
        MultiByteToWideChar(CP_UTF8, 0, srcStr, -1, wstr.get(), len);
        len = WideCharToMultiByte(CP_ACP, 0, wstr.get(), -1, nullptr, 0, nullptr, nullptr);
        std::unique_ptr<char[]> str = std::make_unique<char[]>(len + 1);
        WideCharToMultiByte(CP_ACP, 0, wstr.get(), -1, str.get(), len, nullptr, nullptr);
        return std::string(str.get());
    }
#endif

    template<typename T>
    static std::string join(std::vector<T> list, std::string separator)
    {
        std::stringstream ss;
        for (int i = 0; i < list.size(); i++) {
            if (i != 0) {
                ss << separator;
            }
            ss << list.at(i);
        }
        return ss.str();
    }
};
}

#endif // DATA_INSIGHT_CORE_STRING_UTIL_H
