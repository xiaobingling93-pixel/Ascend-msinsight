/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 */

#ifndef DIC_SOURCE_PROTOCOL_SOURCE_RESPONSE_H
#define DIC_SOURCE_PROTOCOL_SOURCE_RESPONSE_H

#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct SourceCodeFileResBody {
    std::string fileContent;
};

struct SourceCodeFileResponse : public Response {
    SourceCodeFileResponse() : Response(REQ_RES_SOURCE_CODE_FILE) {}

    SourceCodeFileResBody body;
};

struct SourceFileLineRes {
    int line;
    float cycle;
    int instructionExecuted;
    std::vector<std::pair<std::string, std::string>> addressRange;
};

struct SourceApiLineResBody {
    std::vector<SourceFileLineRes> lines;
};

struct SourceApiLineResponse : public Response {
    SourceApiLineResponse() : Response(REQ_RES_SOURCE_API_LINE) {}

    SourceApiLineResBody body;
};

struct SourceApiInstrResBody {
    std::string instructions;
};

struct SourceApiInstrResponse : public Response {
    SourceApiInstrResponse() : Response(REQ_RES_SOURCE_API_INSTRUCTIONS) {}

    SourceApiInstrResBody body;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_SOURCE_PROTOCOL_SOURCE_RESPONSE_H
