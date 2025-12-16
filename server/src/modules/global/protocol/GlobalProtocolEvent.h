/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#ifndef PROFILER_SERVER_GLOBALPROTOCOLEVENT_H
#define PROFILER_SERVER_GLOBALPROTOCOLEVENT_H

#include "ProtocolDefs.h"
#include "ProtocolUtil.h"

namespace Dic::Protocol {
struct ReadFileFailEventBody {
    std::string filePath;
    std::string error;
};

struct ReadFileFailEvent : public Event {
    ReadFileFailEvent() : Event(EVENT_FILES_READ_FAIL) {}
    ReadFileFailEventBody body;
};

void SendReadFileFailEvent(const std::string &filePath, const std::string &errMsg);
}

#endif // PROFILER_SERVER_GLOBALPROTOCOLEVENT_H
