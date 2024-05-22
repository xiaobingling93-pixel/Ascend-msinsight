/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 */

#ifndef DIC_SOURCE_PROTOCOL_SOURCE_REQUEST_H
#define DIC_SOURCE_PROTOCOL_SOURCE_REQUEST_H

#include <string>
#include <optional>
#include <vector>
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct SourceCodeFileParams {
    std::string sourceName;
};

struct SourceCodeFileRequest : public Request {
    SourceCodeFileRequest() : Request(REQ_RES_SOURCE_CODE_FILE) {};
    SourceCodeFileParams params;
};

struct SourceApiLineParams {
    std::string coreName;
    std::string sourceName;
};

struct SourceApiLineRequest : public Request {
    SourceApiLineRequest() : Request(REQ_RES_SOURCE_API_LINE) {};
    SourceApiLineParams params;
};

struct SourceApiInstrRequest : public Request {
    SourceApiInstrRequest() : Request(REQ_RES_SOURCE_API_INSTRUCTIONS) {};
};

struct SourceDetailBaseInfoParams {
};

struct SourceDetailBaseInfoRequest : public Request {
    SourceDetailBaseInfoRequest() : Request(REQ_RES_DETAILS_BASE_INFO) {};
    SourceDetailBaseInfoParams params;
};

struct SourceDetailLoadInfoParams {
};

struct SourceDetailsLoadInfoRequest: public Request {
    SourceDetailsLoadInfoRequest() : Request(REQ_RES_DETAILS_COMPUTE_LOAD_INFO) {};
    SourceDetailLoadInfoParams params;
};

struct DetailsMemoryInfoParams {
    std::string blockId;
};

struct DetailsMemoryGraphRequest: public Request {
    DetailsMemoryGraphRequest() : Request(REQ_RES_DETAILS_COMPUTE_MEMORY_GRAPH) {};
    DetailsMemoryInfoParams params;
};

struct DetailsMemoryTableRequest: public Request {
    DetailsMemoryTableRequest() : Request(REQ_RES_DETAILS_COMPUTE_MEMORY_TABLE) {};
    DetailsMemoryInfoParams params;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_SOURCE_PROTOCOL_SOURCE_REQUEST_H