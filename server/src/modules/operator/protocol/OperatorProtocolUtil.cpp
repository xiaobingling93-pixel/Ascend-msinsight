/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "OperatorProtocolUtil.h"
#include "ProtocolUtil.h"

namespace Dic::Protocol {
    template<>
    std::optional<json_t> ToResponseJson<OperatorCategoryInfoResponse>(const OperatorCategoryInfoResponse &res)
    {
        json_t json;
        ProtocolUtil::SetResponseJsonBaseInfo(res, json);
        json["body"]["data"] = json_t::array();
        for (const OperatorDurationRes& ele : res.datas) {
            json_t dataJson = json_t::object();
            dataJson["name"] = ele.name;
            dataJson["duration"] = ele.duration;
            json["body"]["data"].emplace_back(dataJson);
        }
        return json;
    }

    template<>
    std::optional<json_t> ToResponseJson<OperatorComputeUnitInfoResponse>(const OperatorComputeUnitInfoResponse &res)
    {
        json_t json;
        ProtocolUtil::SetResponseJsonBaseInfo(res, json);
        json["body"]["data"] = json_t::array();
        for (const OperatorDurationRes& ele : res.datas) {
            json_t dataJson = json_t::object();
            dataJson["name"] = ele.name;
            dataJson["duration"] = ele.duration;
            json["body"]["data"].emplace_back(dataJson);
        }
        return json;
    }

    template<>
    std::optional<json_t> ToResponseJson<OperatorStatisticInfoResponse>(const OperatorStatisticInfoResponse &res)
    {
        json_t json;
        ProtocolUtil::SetResponseJsonBaseInfo(res, json);
        json["body"]["total"] = res.total;
        json["body"]["data"] = json_t::array();
        for (const OperatorStatisticInfoRes& ele : res.datas) {
            json_t dataJson = json_t::object();
            dataJson["opType"] = ele.opType;
            dataJson["opName"] = ele.opName;
            dataJson["inputShape"] = ele.inputShape;
            dataJson["accCore"] = ele.accCore;
            dataJson["totalTime"] = ele.totalTime;
            dataJson["count"] = ele.count;
            dataJson["avgTime"] = ele.avgTime;
            dataJson["maxTime"] = ele.maxTime;
            dataJson["minTime"] = ele.minTime;
            json["body"]["data"].emplace_back(dataJson);
        }
        return json;
    }

    template<>
    std::optional<json_t> ToResponseJson<OperatorDetailInfoResponse>(const OperatorDetailInfoResponse &res)
    {
        json_t json;
        ProtocolUtil::SetResponseJsonBaseInfo(res, json);
        json["body"]["total"] = res.total;
        json["body"]["level"] = res.level;
        json["body"]["data"] = json_t::array();
        for (const OperatorDetailInfoRes& ele : res.datas) {
            json_t dataJson = json_t::object();
            dataJson["name"] = ele.name;
            dataJson["type"] = ele.type;
            dataJson["accCore"] = ele.accCore;
            dataJson["startTime"] = ele.startTime;
            dataJson["duration"] = ele.duration;
            dataJson["waitTime"] = ele.waitTime;
            dataJson["blockDim"] = ele.blockDim;
            dataJson["inputShape"] = ele.inputShape;
            dataJson["inputType"] = ele.inputType;
            dataJson["inputFormat"] = ele.inputFormat;
            dataJson["outputShape"] = ele.outputShape;
            dataJson["outputType"] = ele.outputType;
            dataJson["outputFormat"] = ele.outputFormat;
            json["body"]["data"].emplace_back(dataJson);
        }
        return json;
    }

    template<>
    std::optional<json_t> ToResponseJson<OperatorMoreInfoResponse>(const OperatorMoreInfoResponse &res)
    {
        json_t json;
        ProtocolUtil::SetResponseJsonBaseInfo(res, json);
        json["body"]["total"] = res.total;
        json["body"]["level"] = res.level;
        json["body"]["data"] = json_t::array();
        for (const OperatorDetailInfoRes& ele : res.datas) {
            json_t dataJson = json_t::object();
            dataJson["name"] = ele.name;
            dataJson["type"] = ele.type;
            dataJson["accCore"] = ele.accCore;
            dataJson["startTime"] = ele.startTime;
            dataJson["duration"] = ele.duration;
            dataJson["waitTime"] = ele.waitTime;
            dataJson["blockDim"] = ele.blockDim;
            dataJson["inputShape"] = ele.inputShape;
            dataJson["inputType"] = ele.inputType;
            dataJson["inputFormat"] = ele.inputFormat;
            dataJson["outputShape"] = ele.outputShape;
            dataJson["outputType"] = ele.outputType;
            dataJson["outputFormat"] = ele.outputFormat;
            json["body"]["data"].emplace_back(dataJson);
        }
        return json;
    }

    template<>
    std::optional<json_t> ToEventJson<OperatorParseStatusEvent>(const OperatorParseStatusEvent &event)
    {
        json_t json;
        ProtocolUtil::SetEventJsonBaseInfo(event, json);
        json["body"]["rankId"] = event.data.rankId;
        json["body"]["status"] = event.data.status;
        json["body"]["error"] = event.data.error;

        return json;
    }
}
