/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 */
#include "ServerLog.h"
#include "JsonUtil.h"
#include "SourceProtocolUtil.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "SourceProtocol.h"

namespace Dic {
namespace Protocol {
void SourceProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_SOURCE_CODE_FILE, ToCodeFileRequest);
    jsonToReqFactory.emplace(REQ_RES_SOURCE_API_LINE, ToApiLineRequest);
    jsonToReqFactory.emplace(REQ_RES_SOURCE_API_INSTRUCTIONS, ToApiInstrRequest);
}

void SourceProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_SOURCE_CODE_FILE, ToCodeFileResponse);
    resToJsonFactory.emplace(REQ_RES_SOURCE_API_LINE, ToApiLineResponse);
    resToJsonFactory.emplace(REQ_RES_SOURCE_API_INSTRUCTIONS, ToApiInstrResponse);
}

void SourceProtocol::RegisterEventToJsonFuncs()
{
}

#pragma region <<Json To Request>>

std::unique_ptr<Request> SourceProtocol::ToCodeFileRequest(const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<SourceCodeFileRequest> reqPtr = std::make_unique<SourceCodeFileRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.sourceName, json["params"], "sourceName");
    return reqPtr;
}

std::unique_ptr<Request> SourceProtocol::ToApiLineRequest(const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<SourceApiLineRequest> reqPtr = std::make_unique<SourceApiLineRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.coreName, json["params"], "coreName");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.sourceName, json["params"], "sourceName");
    return reqPtr;
}

std::unique_ptr<Request> SourceProtocol::ToApiInstrRequest(const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<SourceApiInstrRequest> reqPtr = std::make_unique<SourceApiInstrRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    return reqPtr;
}

#pragma endregion

#pragma region <<Reponse To Json>>

std::optional<document_t> SourceProtocol::ToCodeFileResponse(const Dic::Protocol::Response &response)
{
    return ToResponseJson<SourceCodeFileResponse>(dynamic_cast<const SourceCodeFileResponse &>(response));
}

std::optional<document_t> SourceProtocol::ToApiLineResponse(const Dic::Protocol::Response &response)
{
    return ToResponseJson<SourceApiLineResponse>(dynamic_cast<const SourceApiLineResponse &>(response));
}

std::optional<document_t> SourceProtocol::ToApiInstrResponse(const Dic::Protocol::Response &response)
{
    return ToResponseJson<SourceApiInstrResponse>(dynamic_cast<const SourceApiInstrResponse &>(response));
}

#pragma endregion

    } // namespace Protocol
} // namespace Dic