/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: implementation for request buffer
 */

#include "ServerLog.h"
#include "ProtocolDefs.h"
#include "ProtocolManager.h"
#include "RegexUtil.h"
#include "ProtocolMessageBuffer.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
ProtocolMessageBuffer::ProtocolMessageBuffer() {}

ProtocolMessageBuffer::~ProtocolMessageBuffer()
{
    Clear();
}

int ProtocolMessageBuffer::GetBodyLength(const uint64_t &headPosition, const uint64_t &headLength) const
{
    std::string lenStr = buffer.substr(headPosition, headLength);
    std::optional<std::smatch> matchRes = RegexUtil::RegexMatch(lenStr, "Content-Length:\\s*(\\d+)");
    if (!matchRes.has_value() || matchRes.value().size() < MATCH_MIN_NUM) {
        return -1;
    }
    int res;
    try {
        res = std::stoi(matchRes.value()[1].str());
    } catch (std::invalid_argument &) {
        res = -1;
    } catch (std::out_of_range &) {
        res = -1;
    } catch (...) {
        res = -1;
    }
    return res;
}

ProtocolMessage::Type ProtocolMessageBuffer::GetMessageType(const std::string &body) const
{
    json_t json;
    try {
        json = json_t::parse(body);
    } catch (json_t::parse_error &) {
        return ProtocolMessage::Type::NONE;
    }
    if (!json.contains("type")) {
        return ProtocolMessage::Type::NONE;
    }
    if (json["type"] == REQUEST_NAME) {
        return ProtocolMessage::Type::REQUEST;
    } else if (json["type"] == RESPONSE_NAME) {
        return ProtocolMessage::Type::RESPONSE;
    } else if (json["type"] == EVENT_NAME) {
        return ProtocolMessage::Type::EVENT;
    } else {
        return ProtocolMessage::Type::NONE;
    }
}

ProtocolMessageBuffer &ProtocolMessageBuffer::operator >> (const std::string &data)
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
    uint64_t splitPos = buffer.find_first_of(REQ_DELIMITER);
    if (splitPos == std::string::npos) {
        return nullptr;
    }
    headLen = splitPos - headPos;
    bodyLen = GetBodyLength(headPos, headLen);
    bodyPos = splitPos + REQ_DELIMITER.length();
    // Prevent integer overflow
    std::string::size_type msgLen = headLen + bodyLen;
    if ((bodyLen == -1) || (buffer.length() < msgLen)) {
        return nullptr;
    }
    std::string bodyStr = buffer.substr(bodyPos, bodyLen);
    buffer = buffer.substr(bodyPos + bodyLen); // buffer removes head and body string
    std::unique_ptr<Request> request = ProtocolManager::Instance().FromJson(bodyStr, error);
    return std::unique_ptr<ProtocolMessage>(request.release());
}

void ProtocolMessageBuffer::Clear()
{
    std::unique_lock<std::mutex> lock(mutex);
    buffer.clear();
}
} // end of namespace Protocol
} // end of namespace Dic