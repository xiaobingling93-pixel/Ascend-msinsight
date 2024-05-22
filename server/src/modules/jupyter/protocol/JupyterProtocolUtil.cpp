/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "JupyterProtocol.h"
#include "JupyterProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
using namespace rapidjson;
#pragma region << Event to json>>

template<typename EVENT>
std::optional <document_t> ToEventJson(const EVENT &event)
{
    return std::nullopt;
}

template<>
std::optional <document_t> ToEventJson<ParseJupyterCompletedEvent>(const ParseJupyterCompletedEvent &event)
        {
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "url", event.body.url, allocator);
    JsonUtil::AddMember(body, "parseResult", event.body.parseResult, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

#pragma endregion
}
}