/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: implementation for request buffer
 */

#include "pch.h"
#include "ProtocolDefs.h"
#include "ProtocolManager.h"
#include "TimelineProtocolRequest.h"
#include "ProtocolMessageBuffer.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;

uint64_t ProtocolMessageBuffer::GetBodyLength(const uint64_t &headPosition, const uint64_t &headLength) const
{
    std::string lenStr = buffer.substr(headPosition, headLength);
    std::optional<std::smatch> matchRes = RegexUtil::RegexMatch(lenStr, "Content-Length:\\s*(\\d+)");
    if (!matchRes.has_value() || matchRes.value().size() < matchMinNum) {
        return invalidBodyLen;
    }
    uint64_t res;
    try {
        res = std::stoull(matchRes.value()[1].str());
    } catch (std::invalid_argument &) {
        res = invalidBodyLen;
    } catch (std::out_of_range &) {
        res = invalidBodyLen;
    } catch (...) {
        res = invalidBodyLen;
    }
    return res;
}

ProtocolMessage::Type ProtocolMessageBuffer::GetMessageType(const std::string &body) const
{
    std::string err;
    auto json = JsonUtil::TryParse(body, err);
    if (!json.has_value()) {
        return ProtocolMessage::Type::NONE;
    }
    if (!json.value().HasMember("type")) {
        return ProtocolMessage::Type::NONE;
    }
    std::string type = json.value()["type"].GetString();
    if (type == REQUEST_NAME) {
        return ProtocolMessage::Type::REQUEST;
    } else if (type == RESPONSE_NAME) {
        return ProtocolMessage::Type::RESPONSE;
    } else if (type == EVENT_NAME) {
        return ProtocolMessage::Type::EVENT;
    } else {
        return ProtocolMessage::Type::NONE;
    }
}

ProtocolMessageBuffer &ProtocolMessageBuffer::operator << (const std::string &data)
{
    std::unique_lock<std::mutex> lock(mutex);
    buffer.append(data);
    return *this;
}

std::unique_ptr<ProtocolMessage> ProtocolMessageBuffer::Pop()
{
    std::unique_lock<std::mutex> lock(mutex);
    headPos = buffer.find(HEAD_START);
    if (headPos == std::string::npos) {
        return nullptr;
    }
    if (headPos != 0) {
        buffer = buffer.substr(headPos);
        headPos = 0;
    }
    uint64_t splitPos = buffer.find(REQ_DELIMITER);
    if (splitPos == std::string::npos) {
        return nullptr;
    }
    headLen = splitPos - headPos;
    bodyLen = GetBodyLength(headPos, headLen);
    bodyPos = splitPos + REQ_DELIMITER.length();
    // Prevent integer overflow
    std::string::size_type msgLen = headLen + bodyLen;
    if ((bodyLen == invalidBodyLen) || (buffer.length() < msgLen)) {
        return nullptr;
    }
    std::string bodyStr = buffer.substr(bodyPos, bodyLen);
    std::unique_ptr<Request> request = ProtocolManager::Instance().FromJson(bodyStr, error);
    auto bodyJson = JsonUtil::TryParse(bodyStr, error).value();

    // 增加上传二进制数据的处理
    uint64_t textLen = 0;
    if (JsonUtil::IsJsonKeyValid(bodyJson, "command") && bodyJson["command"].GetString() == REQ_RES_UPLOAD_FILE &&
        JsonUtil::IsJsonKeyValid(bodyJson, "params") && JsonUtil::IsJsonKeyValid(bodyJson["params"], "textLength")) {
        textLen = bodyJson["params"]["textLength"].GetInt();
        if (buffer.length() < msgLen + textLen) {
            return nullptr;
        }

        auto &uploadFileRequest = dynamic_cast<UploadFileRequest &>(*request);
        uint64_t textPos = bodyPos + bodyLen;
        uploadFileRequest.params.text = buffer.substr(textPos, textLen);
    }

    buffer = buffer.substr(bodyPos + bodyLen + textLen); // buffer removes head and body string
    if (request == nullptr) {
        return nullptr;
    }
    return std::unique_ptr<ProtocolMessage>(request.release());
}

void ProtocolMessageBuffer::Clear()
{
    std::unique_lock<std::mutex> lock(mutex);
    buffer.clear();
}
} // end of namespace Protocol
} // end of namespace Dic