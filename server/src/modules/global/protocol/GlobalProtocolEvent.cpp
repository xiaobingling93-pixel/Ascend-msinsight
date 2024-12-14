/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "GlobalProtocolEvent.h"
#include "WsSender.h"

namespace Dic::Protocol {
void SendReadFileFailEvent(const std::string &filePath, const std::string &errMsg)
{
    auto event = std::make_unique<ReadFileFailEvent>();
    event->body.filePath = filePath;
    event->body.error = errMsg;
    event->moduleName = Protocol::MODULE_GLOBAL;
    event->result = true;
    Dic::SendEvent(std::move(event));
}
}