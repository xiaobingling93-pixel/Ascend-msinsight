/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Regex Utility declaration
 */

#ifndef DATA_INSIGHT_CORE_REGEX_UTIL_H
#define DATA_INSIGHT_CORE_REGEX_UTIL_H

#include <regex>
#include <optional>
#include <string>

namespace Dic {
class RegexUtil {
public:
    static inline bool RegexSearch(const std::string &data, const std::string &pattern)
    {
        std::regex regex(pattern);
        if (!std::regex_search(data, regex)) {
            return false;
        }
        return true;
    }

    static inline std::optional<std::smatch> RegexMatch(const std::string &data, const std::string &pattern)
    {
        std::regex regex(pattern);
        std::smatch result;
        if (!std::regex_match(data, result, regex)) {
            return std::nullopt;
        }
        return result;
    }
};
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_REGEX_UTIL_H
