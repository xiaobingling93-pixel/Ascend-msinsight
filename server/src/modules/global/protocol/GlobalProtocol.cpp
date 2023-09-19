/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "GlobalProtocolRequest.h"
#include "GlobalProtocolResponse.h"
#include "GlobalProtocolUtil.h"
#include "GlobalProtocol.h"

namespace Dic {
namespace Protocol {
void GlobalProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_TOKEN_CREATE, ToTokenCreateRequest);
    jsonToReqFactory.emplace(REQ_RES_TOKEN_DESTROY, ToTokenDestroyRequest);
    jsonToReqFactory.emplace(REQ_RES_TOKEN_CHECK, ToTokenCheckRequest);
    jsonToReqFactory.emplace(REQ_RES_FILES_GET, ToFilesGetRequest);
}

void GlobalProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_TOKEN_CREATE, ToTokenCreateResponseJson);
    resToJsonFactory.emplace(REQ_RES_TOKEN_DESTROY, ToTokenDestroyResponseJson);
    resToJsonFactory.emplace(REQ_RES_TOKEN_CHECK, ToTokenCheckResponseJson);
    resToJsonFactory.emplace(REQ_RES_FILES_GET, ToFilesGetResponseJson);
}

void GlobalProtocol::RegisterEventToJsonFuncs()
{
}

#pragma region <<Json To Request>>
std::unique_ptr<Request> GlobalProtocol::ToTokenCreateRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<TokenCreateRequest> reqPtr = std::make_unique<TokenCreateRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (JsonUtil::IsJsonKeyValid(json, "params")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.deadTime, json["params"], "deadTime");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.parentToken, json["params"], "parentToken");
    }
    return reqPtr;
}

std::unique_ptr<Request> GlobalProtocol::ToTokenDestroyRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<TokenDestroyRequest> reqPtr = std::make_unique<TokenDestroyRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.destroyToken, json["params"], "destroyToken");
    return reqPtr;
}

std::unique_ptr<Request> GlobalProtocol::ToTokenCheckRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<TokenCheckRequest> reqPtr = std::make_unique<TokenCheckRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.checkedToken, json["params"], "checkedToken");
    return reqPtr;
}

std::unique_ptr<Request> GlobalProtocol::ToFilesGetRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<FilesGetRequest> reqPtr = std::make_unique<FilesGetRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.path, json["params"], "path");
    return reqPtr;
}
#pragma endregion

#pragma region <<Response To Json>>
std::optional<json_t> GlobalProtocol::ToTokenCreateResponseJson(const Response &response)
{
    return ToResponseJson<TokenCreateResponse>(dynamic_cast<const TokenCreateResponse &>(response));
}

std::optional<json_t> GlobalProtocol::ToTokenDestroyResponseJson(const Response &response)
{
    return ToResponseJson<TokenDestroyResponse>(dynamic_cast<const TokenDestroyResponse &>(response));
}

std::optional<json_t> GlobalProtocol::ToTokenCheckResponseJson(const Response &response)
{
    return ToResponseJson<TokenCheckResponse>(dynamic_cast<const TokenCheckResponse &>(response));
}

std::optional<json_t> GlobalProtocol::ToFilesGetResponseJson(const Response &response)
{
    return ToResponseJson<FilesGetResponse>(dynamic_cast<const FilesGetResponse &>(response));
}
#pragma endregion

#pragma region <<Event To Json>>
#pragma endregion
} // namespace Protocol
} // namespace Dic
