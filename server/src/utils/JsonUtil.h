/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Json Utility declaration
 */

#ifndef DATA_INSIGHT_CORE_JSON_UTIL_H
#define DATA_INSIGHT_CORE_JSON_UTIL_H

#include <string>
#include <optional>
#include <vector>
#include "GlobalDefs.h"
#include "ServerLog.h"

namespace Dic {
class JsonUtil {
public:
    static inline bool IsJsonKeyValid(const json_t &json, std::string_view key)
    {
        return json.HasMember(key.data()) && !json[key.data()].IsNull();
    }

    template <typename T> static inline void SetByJsonKeyValue(T &src, const json_t &json, std::string_view key)
    {
        SetByJsonKeyValueHelper(src, json, key);
    }

    template <typename T> static inline void AddMember(json_t &json, std::string_view key, T &&value,
        RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        AddMemberHelper(json, key, std::forward<T>(value), allocator);
    }

    template <typename T> static inline void AddConstMember(json_t &json, std::string_view key, T &&value,
                                                       RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        AddConstMemberHelper(json, key, std::forward<T>(value), allocator);
    }

    template <typename T> static inline void AddMember(json_t &json, std::string_view key, T &value,
                                                       RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        AddMemberHelper(json, key, value, allocator);
    }

    template <typename T> static inline void AddMember(json_t &json, std::string_view key, std::vector<T> value,
                                                       RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        json_t temp(kArrayType);
        for (const T item: value) {
            if constexpr (std::is_same_v<T, std::string>) {
                temp.PushBack(json_t().SetString(item.c_str(), allocator), allocator);
            } else {
                temp.PushBack(json_t().SetString(std::to_string(item).c_str(), allocator), allocator);
            }
        }
        AddMemberHelper(json, key, temp, allocator);
    }

    static inline std::optional<document_t> TryParse(const std::string &jsonStr, std::string &error)
    {
        return TryParse<kParseDefaultFlags>(jsonStr, error);
    }

    template <unsigned parseFlags>
    static inline std::optional<document_t> TryParse(const std::string &jsonStr, std::string &error)
    {
        document_t doc;
        doc.Parse<parseFlags>(jsonStr.c_str(), jsonStr.length());
        if (doc.HasParseError()) {
            static const size_t PRINT_ERROR_SIZE = 10;
            auto offset = doc.GetErrorOffset();
            auto start = offset >= PRINT_ERROR_SIZE ? offset - PRINT_ERROR_SIZE : 0;
            error = "Error code:" + std::to_string(doc.GetParseError()) +
                ". str:" + jsonStr.substr(start, offset - start + PRINT_ERROR_SIZE);
            return std::nullopt;
        }
        return std::make_optional(std::move(doc));
    }

    static inline bool IsJsonArray(const json_t &json, std::string_view key)
    {
        return (json.HasMember(key.data()) && json[key.data()].IsArray());
    }

    template <typename T>
    static inline std::vector<T> GetVector(const json_t &json, std::string_view key)
    {
        std::vector<T> vec;
        if (!IsJsonArray(json, key)) {
            return vec;
        }
        const Value& array = json[key.data()];
        vec.reserve(array.Size());
        for (auto &item : array.GetArray()) {
            if constexpr (std::is_same_v<T, double>) {
                vec.push_back(item.GetDouble());
            } else if constexpr (std::is_same_v<T, std::string>) {
                vec.push_back(item.GetString());
            } else {
                Server::ServerLog::Error("Get vector from json error: unsupported type!");
            }
        }
        return vec;
    }

    static inline std::string JsonDump(const json_t &json)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        json.Accept(writer);
        return buffer.GetString();
    }

    // for timeline file parse
    static inline double GetDouble(const json_t &json, std::string_view key)
    {
        if (!json.HasMember(key.data())) {
            return 0;
        }
        if (json[key.data()].IsNumber()) {
            return json[key.data()].GetDouble();
        }
        if (json[key.data()].IsString()) {
            try {
                return std::stod(json[key.data()].GetString());
            } catch (std::exception &e) {
                return 0;
            }
        }
        return 0;
    }

    static inline long double GetLongDouble(const json_t &json, std::string_view key)
    {
        if (!json.HasMember(key.data())) {
            return 0;
        }
        if (json[key.data()].IsNumber()) {
            return json[key.data()].GetDouble();
        }
        if (json[key.data()].IsString()) {
            try {
                return std::stold(json[key.data()].GetString());
            } catch (std::exception &e) {
                return 0;
            }
        }
        return 0;
    }

    static inline int64_t GetInteger(const json_t &json, std::string_view key)
    {
        if (!json.HasMember(key.data())) {
            return 0;
        }
        if (json[key.data()].IsInt()) {
            return json[key.data()].GetInt64();
        }
        if (json[key.data()].IsDouble()) {
            return static_cast<int64_t>(json[key.data()].GetDouble());
        }
        if (json[key.data()].IsString()) {
            try {
                return std::stoll(json[key.data()].GetString());
            } catch (std::exception &e) {
                return 0;
            }
        }
        return 0;
    }

    static inline std::string GetString(const json_t &json, std::string_view key)
    {
        if (json.HasMember(key.data()) && json[key.data()].IsString()) {
            return json[key.data()].GetString();
        }
        return "";
    }

    static inline std::optional<std::string> GetOptionalString(const json_t &json, std::string_view key)
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

    static inline std::string GetDumpString(const json_t &json, std::string_view key)
    {
        if (!json.HasMember(key.data())) {
            return "";
        }
        if (json[key.data()].IsString()) {
            return json[key.data()].GetString();
        } else {
            return JsonDump(json[key.data()]);
        }
    }

    static std::vector<std::string> JsonToVector(const std::string& jsonStr)
    {
        rapidjson::Document document;
        document.Parse(jsonStr.c_str());
        if (!document.IsArray()) {
            return {};
        }
        std::vector<std::string> result;
        for (rapidjson::SizeType i = 0; i < document.Size(); i++) {
            if (document[i].IsString()) {
                result.emplace_back(document[i].GetString());
            }
        }
        return result;
    }

private:
    template <typename T> static inline void SetByJsonKeyValueHelper(std::optional<T> &src, const json_t &json,
        std::string_view key)
    {
        SetByJsonKeyValueHelper(*src, json, key);
    }

    template <typename T> static inline void SetByJsonKeyValueHelper(T &src, const json_t &json, std::string_view key)
    {
        if (json.HasMember(key.data()) && json[key.data()].Is<T>()) {
            src = json[key.data()].Get<T>();
        }
    }

    static inline void SetByJsonKeyValueHelper(std::string &src, const json_t &json, std::string_view key)
    {
        if (json.HasMember(key.data()) && json[key.data()].IsString()) {
            src = json[key.data()].GetString();
        }
    }

    template <typename T> static inline void AddMemberHelper(json_t &json, std::string_view key, T &&value,
        RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        json.AddMember(rapidjson::StringRef(key.data(), key.length()), std::forward<T>(value), allocator);
    }

    static inline void AddMemberHelper(json_t &json, std::string_view key, std::string &&value,
                                       RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        json.AddMember(rapidjson::StringRef(key.data(), key.length()),
                       json_t().SetString(value.data(), value.length(), allocator), allocator);
    }

    static inline void AddMemberHelper(json_t &json, std::string_view key, const std::string &&value,
                                       RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        json.AddMember(rapidjson::StringRef(key.data(), key.length()),
                       json_t().SetString(value.c_str(), value.length(), allocator), allocator);
    }

    template <typename T> static inline void AddMemberHelper(json_t &json, std::string_view key, T &value,
        RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        json.AddMember(rapidjson::StringRef(key.data(), key.length()), value, allocator);
    }

    static inline void AddMemberHelper(json_t &json, std::string_view key, std::string &value,
                                       RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        json.AddMember(rapidjson::StringRef(key.data(), key.length()),
                       json_t().SetString(value.data(), value.length(), allocator), allocator);
    }

    static inline void AddMemberHelper(json_t &json, std::string_view key, const std::string &value,
                                       RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        json.AddMember(rapidjson::StringRef(key.data(), key.length()),
                       json_t().SetString(value.c_str(), value.length(), allocator), allocator);
    }

    static inline void AddConstMemberHelper(json_t &json, std::string_view key, const std::string &value,
                                       RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        std::string strKey{key};
        json.AddMember(Value(strKey.c_str(), allocator),
                       json_t().SetString(value.c_str(), value.length(), allocator), allocator);
    }
};
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_JSON_UTIL_H
