/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MemoryProtocolUtil.h"
#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
#pragma region <<Response to json>>

template <> std::optional<json_t> ToResponseJson<MemoryOperatorResponse>(const MemoryOperatorResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["operatorDetail"] = json_t::array();
    for (const MemoryOperator& anOperator : response.operatorDetails) {
        json_t basicJson = json_t::object();
        basicJson["name"] = anOperator.name;
        basicJson["size"] = anOperator.size;
        basicJson["allocationTime"] = anOperator.allocationTime;
        basicJson["releaseTime"] = anOperator.releaseTime;
        basicJson["duration"] = anOperator.duration;
        json["body"]["operatorDetail"].emplace_back(basicJson);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<MemoryViewResponse>(const MemoryViewResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["AllocatesLine"] = response.map.allocatesLine;
    json["body"]["ReservedLine"] = response.map.reservedLine;
    json["body"]["AppLine"] = response.map.appLine;
    json["body"]["peakMemoryUsage"] = response.map.peakMemoryUsage;
    return json;
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic