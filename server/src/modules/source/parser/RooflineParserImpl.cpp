// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#include "RooflineParserImpl.h"
#include "SourceFileParser.h"
namespace Dic::Module::Source {
using namespace Server;
using namespace Protocol;
bool RooflineParserImpl::GetDetailsRoofline(const std::string& jsonStr, Protocol::DetailsRooflineBody &responseBody)
{
    std::optional<Protocol::RooflineData> rooflineData = ConvertStrToRooflineData(jsonStr);
    if (rooflineData.has_value()) {
        responseBody.advice = rooflineData->advice;
        responseBody.data = std::move(rooflineData->multipleRooflines);
        return true;
    }
    return false;
}

std::optional<Protocol::RooflineData> RooflineParserImpl::ConvertStrToRooflineData(const std::string &json)
{
    if (json.empty()) {
        return std::nullopt;
    }
    Protocol::RooflineData rooflineData;
    std::string error;
    auto jsonParsed = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(json, error);
    if (!error.empty()) {
        ServerLog::Error("Illegal roofline data which can't parse into json, error:", error);
        return std::nullopt;
    }
    rooflineData.advice = JsonUtil::GetString(jsonParsed.value(), "advice");
    if (!jsonParsed.value().HasMember("multiple_rooflines")) {
        ServerLog::Error("Not have roofline graph part");
        return std::nullopt;
    }
    Value &multiRoofline = jsonParsed.value()["multiple_rooflines"];
    if (!multiRoofline.IsArray()) {
        ServerLog::Error("Invaild data format, roofline graph should be array");
        return std::nullopt;
    }
    std::transform(multiRoofline.GetArray().begin(), multiRoofline.GetArray().end(),
                   std::back_inserter(rooflineData.multipleRooflines),
                   RooflineParserImpl::ParseRooflineData);
    return rooflineData;
}

Protocol::RooflineGraph RooflineParserImpl::ParseRooflineData(const json_t &item)
{
    Protocol::RooflineGraph rooflineData;
    rooflineData.title = JsonUtil::GetString(item, "title");
    if (!item.HasMember("rooflines")) {
        ServerLog::Warn("Not have roofline data");
        return rooflineData;
    }
    const Value &rooflines = item["rooflines"];
    if (!rooflines.IsArray()) {
        return rooflineData;
    }
    std::transform(rooflines.GetArray().begin(), rooflines.GetArray().end(),
                   std::back_inserter(rooflineData.rooflines), RooflineParserImpl::ParseRoofline);
    return rooflineData;
}

Protocol::Roofline RooflineParserImpl::ParseRoofline(const json_t &item)
{
    constexpr int precision = 2;
    Protocol::Roofline roofline;
    roofline.bw = NumberUtil::StrReservedNDigits(JsonUtil::GetString(item, "bw"), precision);
    roofline.bwName = JsonUtil::GetString(item, "bw_name");
    roofline.computility = NumberUtil::StrReservedNDigits(JsonUtil::GetString(item, "computility"), precision);
    roofline.computilityName = JsonUtil::GetString(item, "computility_name");
    auto point = JsonUtil::GetVector<std::string>(item, "point");
    constexpr int pointSizeLimit = 2;
    if (point.size() == pointSizeLimit) {
        roofline.point = {NumberUtil::StrReservedNDigits(point[0], precision),
                          NumberUtil::StrReservedNDigits(point[1], precision)};
    }
    roofline.ratio = JsonUtil::GetString(item, "ratio");
    return roofline;
}
} // end of Dic::Module::Source