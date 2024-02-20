/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "SourceProtocol.h"
#include "SourceProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
using namespace rapidjson;
#pragma region <<Response to json>>

template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("ToResponseJson is not implemented. command:", response.command);
    return std::nullopt;
}

template<> std::optional<document_t> ToResponseJson<SourceCodeFileResponse>(const SourceCodeFileResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "fileContent", response.body.fileContent, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<> std::optional<document_t> ToResponseJson<SourceApiLineResponse>(const SourceApiLineResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);

    json_t lines(kArrayType);
    for (auto lineRes: response.body.lines) {
        json_t line(kObjectType);
        JsonUtil::AddMember(line, "Line", lineRes.line, allocator);
        JsonUtil::AddMember(line, "Instruction Executed", lineRes.instructionExecuted, allocator);
        JsonUtil::AddMember(line, "Cycle", lineRes.cycle, allocator);
        json_t ranges(kArrayType);
        for (auto pair: lineRes.addressRange) {
            json_t range(kArrayType);
            range.PushBack(json_t().SetString(pair.first.c_str(), allocator), allocator);
            range.PushBack(json_t().SetString(pair.second.c_str(), allocator), allocator);
            ranges.PushBack(range, allocator);
        }
        JsonUtil::AddMember(line, "Address Range", ranges, allocator);
        lines.PushBack(line, allocator);
    }
    JsonUtil::AddMember(body, "lines", lines, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<> std::optional<document_t> ToResponseJson<SourceApiInstrResponse>(const SourceApiInstrResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "instructions", response.body.instructions, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic