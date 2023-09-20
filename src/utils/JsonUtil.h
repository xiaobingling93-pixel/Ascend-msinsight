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

    static inline bool IsJsonArray(const json_t &json, const std::string &key)
    {
        return (json.contains(key) && json.at(key).is_array());
    }

    // rapidjson
    static inline std::string JsonDump(const rapidjson::Value &json)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        json.Accept(writer);
        return buffer.GetString();
    }

    static inline double GetDouble(const rapidjson::Value &json, std::string_view key)
    {
        if (json.HasMember(key.data()) && json[key.data()].IsDouble()) {
            return json[key.data()].GetDouble();
        }
        return 0;
    }

    static inline int64_t GetInteger(const rapidjson::Value &json, std::string_view key)
    {
        if (json.HasMember(key.data()) && json[key.data()].IsInt64()) {
            return json[key.data()].GetInt64();
        }
        return 0;
    }

    static inline std::string GetString(const rapidjson::Value &json, std::string_view key)
    {
        if (json.HasMember(key.data()) && json[key.data()].IsString()) {
            return json[key.data()].GetString();
        }
        return "";
    }

    static inline std::optional<std::string> GetOptionalString(const rapidjson::Value &json, std::string_view key)
    {
        if (!json.HasMember(key.data())) {
            return std::nullopt;
        }
        if (json[key.data()].IsString()) {
            return json[key.data()].GetString();
        } else {
            return JsonDump(json[key.data()]);
        }
    }
};
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_JSON_UTIL_H
