/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#include "CommonRequests.h"
namespace Dic::Protocol {
bool FiltersParam::SetFiltersFromJson(const json_t& json, const std::vector<TableViewColumn>& columns,
                                      std::string& errorMsg)
{
    if (!json.IsObject()) {
        errorMsg = "Failed to set filters params from param json object: json is null or type invalid";
        return false;
    }
    if (!json.HasMember("filters")) {
        return true;
    }
    const json_t& filtersJson = json["filters"];
    if (!filtersJson.IsObject()) {
        errorMsg = "Failed to set filters params from param json object: filter json is null or type invalid";
        return false;
    }
    for (auto member = filtersJson.MemberBegin(); member != filtersJson.MemberEnd(); member++) {
        auto &key  = member->name;
        auto &value = member->value;
        if (!key.IsString() || !value.IsString()) {
            errorMsg = "Failed to set filters params from param json object: invalid type";
            return false;
        }
        std::string strKey = key.GetString();
        std::string strValue = value.GetString();
        auto const columnIt = FindColumnByKey(strKey, columns);
        if (columnIt == columns.end()) {
            errorMsg = StringUtil::FormatString("Invalid filter, detail: Non-exist column '{}'", strKey);
            return false;
        }
        if (!columnIt->searchable) {
            errorMsg = StringUtil::FormatString("Invalid filter, detail: column '{}' is not searchable", strKey);
            return false;
        }
        filters.emplace(std::string(columnIt->key), strValue);
    }
    return true;
}

bool OrderByParam::SetOrderFromJson(const json_t& json, const std::vector<TableViewColumn>& columns,
                                    std::string& errorMsg)
{
    if (!json.IsObject()) {
        errorMsg = "Failed to set orderBy param from param json object: json is null or type invalid";
        return false;
    }
    if (!json.HasMember("orderBy")) {
        return true;
    }
    const json_t &orderByJson = json["orderBy"];
    if (orderByJson.IsNull() || !orderByJson.IsString()) {
        errorMsg = "Failed to set orderBy param from param json object: orderBy is null or type invalid";
        return false;
    }
    std::string orderByStr = orderByJson.GetString();
    if (orderByStr.empty()) {
        return true;
    }
    auto const columnIt = FindColumnByKey(orderByStr, columns);
    if (columnIt == columns.end()) {
        errorMsg = "Failed to set orderBy param from param json object: Non-exist column " + orderByStr;
        return false;
    }
    if (!columnIt->sortable) {
        errorMsg = StringUtil::FormatString("Invalid order, detail: column '{}' is not sortable",
                                            columnIt->name);
        return false;
    }
    orderBy = StringUtil::FormatString(std::string(columnIt->key));
    std::string order;
    JsonUtil::SetByJsonKeyValue(order, json, "order");
    if (!order.empty()) {
        desc = order == "descend";
        return true;
    }
    JsonUtil::SetByJsonKeyValue(desc, json, "desc");
    return true;
}

bool RangeFiltersParam::SetRangeFiltersFromJson(const json_t& json, const std::vector<TableViewColumn>& columns,
                                                std::string& errorMsg)
{
    if (!json.IsObject()) {
        errorMsg = "Failed to set range filters params from param json object: json is null or type invalid";
        return false;
    }
    // 没有rangeFilters字段或rangeFilters为空代表不做范围过滤，合法
    if (!json.HasMember("rangeFilters") || json["rangeFilters"].MemberCount() == 0) {
        return true;
    }
    const json_t& rangeFiltersJson = json["rangeFilters"];
    if (!rangeFiltersJson.IsObject()) {
        errorMsg = "Failed to set range filters params from param json object: filter json is null or type invalid";
        return false;
    }
    for (auto member = rangeFiltersJson.MemberBegin(); member != rangeFiltersJson.MemberEnd(); member++) {
        auto &key  = member->name;
        auto &value = member->value;
        if (!key.IsString() || !value.IsArray()) {
            errorMsg = "Failed to set range filters params from param json object: format error.";
            return false;
        }
        std::string strKey = key.GetString();
        auto rangeJson = value.GetArray();
        if (rangeJson.Size() != 2 || !rangeJson[0].IsNumber() || !rangeJson[1].IsNumber()) {
            errorMsg = StringUtil::FormatString("Invalid range array size or range array type.");
            return false;
        }
        std::pair<double, double> rangePair;
        rangePair.first = rangeJson[0].GetDouble();
        rangePair.second = rangeJson[1].GetDouble();
        auto const columnIt = FindColumnByKey(strKey, columns);
        if (columnIt == columns.end()) {
            errorMsg = StringUtil::FormatString("Invalid filter, detail: Non-exist column '{}'", strKey);
            return false;
        }
        if (!columnIt->rangeFilterable) {
            errorMsg = StringUtil::FormatString("Invalid range filter, "
                                                "detail: column '{}' is not range filterable", strKey);
            return false;
        }
        rangeFilters.emplace(StringUtil::FormatString("{}", columnIt->key), rangePair);
    }
    return true;
}

document_t TableViewColumn::ToTableHeaderJson(MemoryPoolAllocator<CrtAllocator>& allocator) const
{
    document_t headerJson(kObjectType);
    std::string headerName = std::string(name);
    StringUtil::StripDbColumnName(headerName);
    JsonUtil::AddMember(headerJson, "name", headerName, allocator);
    JsonUtil::AddMember(headerJson, "key", std::string(key), allocator);
    JsonUtil::AddMember(headerJson, "sortable", sortable, allocator);
    JsonUtil::AddMember(headerJson, "searchable", searchable, allocator);
    JsonUtil::AddMember(headerJson, "rangeFilterable", rangeFilterable, allocator);
    return headerJson;
}
}