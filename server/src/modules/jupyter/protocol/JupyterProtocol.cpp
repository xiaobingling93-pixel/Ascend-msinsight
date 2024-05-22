/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "JupyterProtocol.h"
#include "JupyterProtocolUtil.h"
namespace Dic {
namespace Protocol {
void JupyterProtocol::RegisterJsonToRequestFuncs()
{
}

void JupyterProtocol::RegisterResponseToJsonFuncs()
{
}

void JupyterProtocol::RegisterEventToJsonFuncs()
{
    eventToJsonFactory.emplace(EVENT_PARSE_JUPYTER_COMPLETED, ToParseJupyterCompletedEventJson);
}

#pragma region << Event To Json>>

std::optional<document_t> JupyterProtocol::ToParseJupyterCompletedEventJson(const Event &event)
{
    return ToEventJson<ParseJupyterCompletedEvent>(dynamic_cast<const ParseJupyterCompletedEvent &>(event));
}

#pragma endregion
}
}