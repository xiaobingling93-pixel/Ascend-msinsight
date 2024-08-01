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
#include <sstream>
#include <chrono>
#include <zlib.h>
#include "algorithm"
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <map>
#include <iomanip>

namespace Dic {
class StringUtil {
public:
    static inline const std::string IntToString(int number, int length)
    {
        std::string s = std::to_string(number);
        if (length > s.size()) {
            s.insert(0, length - s.size(), '0');
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
        for (char c : str) {
            byte = static_cast<std::uint8_t>(c);
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
        static const size_t MIN_LEN = 3;
        if (str.length() < MIN_LEN) {
            return str;
        }
        std::string res(str);
        int pos = res.length() / MIN_LEN;
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

static inline const std::wstring String2WString(const std::string& s)
{
    const size_t bufferSize = s.size() + 1;
    if (bufferSize > PATH_MAX) { // 超过最大长度返回空字符串
        return std::wstring();
    }
    wchar_t dstWstr[PATH_MAX] = {0};
    mbstowcs(dstWstr, s.c_str(), bufferSize);
    std::wstring result = dstWstr;
    return result;
}

static inline const std::string WString2String(const std::wstring& ws)
{
    size_t bufferSize = ws.size() * 4 + 1;
    if (ws.size() + 1 > PATH_MAX) {
        return std::string();
    }
    char dstStr[PATH_MAX] = {0};
    wcstombs(dstStr, ws.c_str(), bufferSize);
    std::string result = dstStr;
    return result;
}

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

    static std::string Join4SqlGroup(std::vector<std::string> list)
    {
        std::stringstream ss;
        for (int i = 0; i < list.size(); i++) {
            if (list.at(i).empty()) {
                continue;
            }
            if (i != 0) {
                ss << ",";
            }
            ss << ("'" + list.at(i) + "'");
        }
        return ss.str();
    }

    static bool StartWith(const std::string& str, const std::string& start)
    {
        int srcLen = str.size();
        int startLen = start.size();
        return srcLen >= startLen && str.substr(0, startLen) == start;
    }

    static bool EndWith(const std::string& str, const std::string& suffix)
    {
        if (str.length() < suffix.length()) {
            return false;
        }
        return str.substr(str.length() - suffix.length()) == suffix;
    }

    static bool Contains(const std::string& str, const std::string& subStr)
    {
        return str.find(subStr) != std::string::npos;
    }

    /**
     * 替换第一个对应的字符串
     * @param target 目标字符串
     * @param replaceStr 被替换的字符串
     * @param subStr 用于替换的字符串
     * @return true / false
     */
    static std::string ReplaceFirst(const std::string& target, const std::string& replaceStr, const std::string& subStr)
    {
        auto result = std::string(target);
        auto index = target.find(replaceStr);
        if (index == std::string::npos) {
            return result;
        }
        return result.replace(index, replaceStr.length(), subStr);
    }

    /**
     * 校验命令行路径参数
     * @param path 路径
     * @return true / false
     */
    static bool ValidateCommandFilePathParam(const std::string& path)
    {
        if (path.empty()) {
            return false;
        }
#ifdef _WIN32
        char injectList[] = {'|', ';', '&', '$', '>', '<', '`', '!', '\n'};
#else
        char injectList[] = {'|', ';', '&', '$', '>', '<', '`', '\\', '!', '\n'};
#endif
        for (const auto &ch: path) {
            if (std::find(std::begin(injectList), std::end(injectList), ch) != std::end(injectList)) {
                return false;
            }
        }
        return true;
    }

static std::vector<std::string> StringSplit(const std::string& str)
{
    std::vector<std::string> result;
    std::string subStr = "";
    int count = 0;
    for (char ch : str) {
        // 根据字符串内 ” 的数量来判断是否是一个完整的字符串，count % 2 = 0 为偶数个，满足要求
        if (ch == ',' and count % 2 == 0) {
            if (count != 0) {
                subStr = '\"' + subStr + '\"';
            }
            result.push_back(subStr);
            subStr = "";
            count = 0;
        } else if (ch == '\"') {
            count++;
        } else {
            subStr += ch;
        }
    }
    result.push_back(subStr);
    return result;
}

static std::string ToCamelCase(const std::string& str)
{
    std::string res;
    if (str.empty()) {
        return res;
    }
    // 将字符串使用“_”进行分割
    std::vector<std::string> strList = StringUtil::Split(str, "_");
    if (strList.empty()) {
        return res;
    }
    res = strList[0];
    // 对第二个子串开始，如果子串不为空，则将子串的首字母变成大写
    for (int i = 1; i < strList.size(); ++i) {
        if (!strList[i].empty()) {
            strList[i][0] = std::toupper(strList[i][0]);
            res += strList[i];
        }
    }
    return res;
}

static std::string GetHashStrName(const std::string &string)
{
    std::hash<std::string> hasher;
    std::size_t fileHash = hasher(string);
    std::string fileHashStr = std::to_string(fileHash);

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t timestamp = std::chrono::system_clock::to_time_t(now);
    std::string timestampStr = std::to_string(timestamp);

    std::string dbName =
            fileHashStr.substr(0, 5) + "_" + timestampStr.substr(timestampStr.length() - 5);
    return dbName;
}

static bool CheckSqlValid(const std::string& input)
{
    std::string pattern = "[a-zA-Z0-9_-]";
    std::regex regex(pattern);
    // 该方法用于处理SQL参数，当前只验证字母数字下划线中划线，后续可以兼容扩展
    std::string result;
    for (char c : input) {
        if (!std::regex_match(std::string(1, c), regex)) {
            return false;
        }
    }
    return true;
}

static bool GetRegularMatchPositions(const std::string &input, const std::regex &regex,
                                            std::vector<int64_t> &positions)
{
    if (input.empty()) {
        return false;
    }
    std::string::const_iterator it = input.cbegin();
    std::string::const_iterator end = input.cend();
    std::regex_iterator<std::string::const_iterator> iter(it, end, regex);
    std::regex_iterator<std::string::const_iterator> enditer;
    std::regex_iterator<std::string::const_iterator> matchIter;
    while (iter != enditer) {
        matchIter = iter;
        std::smatch match = *matchIter;
        positions.emplace_back(match.position());
        ++iter;
    }
    return true;
}

inline static std::string DoubleToStringWithTwoDecimalPlaces(double value)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << value; // 保留有效位数为2，四舍五入
    return stream.str();
}

static std::optional<std::string> Decompress(const std::string &str)
{
    // 非 zlib 格式压缩数据直接返回
    if (str.empty() || str[0] != '\x78') {
        return str;
    }
    // 解压后的数据不能大于50M
    const static size_t MAX_DECOMPRESSED_SIZE = 1024 * 1024 * 50;

    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = 0;
    zs.next_in = Z_NULL;

    if (inflateInit2(&zs, MAX_WBITS) != Z_OK) {
        return std::nullopt;
    }

    zs.next_in = (Bytef *)str.data();
    zs.avail_in = str.size();

    int ret;
    char buffer[32768];
    std::string output;

    do {
        zs.next_out = reinterpret_cast<Bytef *>(buffer);
        zs.avail_out = sizeof(buffer);

        ret = inflate(&zs, 0);

        if (output.size() < zs.total_out) {
            if (zs.total_out > MAX_DECOMPRESSED_SIZE) {
                inflateEnd(&zs);
                return std::nullopt;
            }
            output.append(buffer, zs.next_out - reinterpret_cast<Bytef *>(buffer));
        }
    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        return std::nullopt;
    }

    return output;
}
};
}

#endif // DATA_INSIGHT_CORE_STRING_UTIL_H
