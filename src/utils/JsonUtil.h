/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Json Utility declaration
 */

#ifndef DATA_INSIGHT_CORE_JSON_UTIL_H
#define DATA_INSIGHT_CORE_JSON_UTIL_H

#include <string>
#include <optional>
#include "GlobalDefs.h"

namespace Dic {
class JsonUtil {
public:
    static inline bool IsJsonKeyValid(const json_t &json, const std::string &key)
    {
        return json.contains(key) && !json[key].is_null();
    }

    template <typename T> static inline void SetByJsonKeyValue(T &src, const json_t &json, const std::string &key)
    {
        if (json.contains(key) && !json[key].is_null()) {
            src = json[key];
        }
    }

    template <typename T> static inline void SetByOptionalValue(const T &src, json_t &json, const std::string &key)
    {
        if (src.has_value()) {
            json[key] = src.value();
        }
    }

    static inline std::optional<json_t> TryParse(const std::string &jsonStr, std::string &error)
    {
        try {
            return json_t::parse(jsonStr);
        } catch (json_t::parse_error &) {
            error = "Failed to parse json string.";
        } catch (json_t::type_error &) {
            error = "Failed to parse json type.";
        } catch (...) {
            error = "Unknown parse error.";
        }
        return std::nullopt;
    }

    static inline std::optional<json_t> TryParse(char *buffer, uint64_t length, std::string &error)
    {
        try {
            return json_t::parse(buffer, buffer + length);
        } catch (json_t::parse_error &) {
            error = "Failed to parse json string.";
        } catch (json_t::type_error &) {
            error = "Failed to parse json type.";
        } catch (...) {
            error = "Unknown parse error.";
        }
        return std::nullopt;
    }

    static inline bool IsJsonArray(const json_t &json, const std::string &key)
    {
        return (json.contains(key) && json.at(key).is_array());
    }

    static inline std::string GetString(const json_t &json, const std::string &key)
    {
        if (json.contains(key) && json.at(key).is_string()) {
            return json.at(key);
        }
        return "";
    }

    static inline std::optional<std::string> GetOptionalString(const json_t &json, const std::string &key)
    {
        if (!json.contains(key)) {
            return std::nullopt;
        }
        if (json.at(key).is_string()) {
            return json.at(key);
        } else {
            return nlohmann::to_string(json.at(key));
        }
    }

    static inline int64_t GetInteger(const json_t &json, const std::string &key)
    {
        if (json.contains(key) && json.at(key).is_number()) {
            return json.at(key);
        }
        return 0;
    }

    static inline double GetDouble(const json_t &json, const std::string &key)
    {
        if (json.contains(key) && json.at(key).is_number()) {
            return json.at(key);
        }
        return 0;
    }
};
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_JSON_UTIL_H
