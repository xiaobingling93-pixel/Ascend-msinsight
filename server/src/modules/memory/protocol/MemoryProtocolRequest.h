/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORYPROTOCOLREQUEST_H
#define PROFILER_SERVER_MEMORYPROTOCOLREQUEST_H

#include <string>
#include <optional>
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {

struct MemoryOperatorParams {
    std::string rankId;
    std::string type;
    std::string searchName;
    int64_t minSize;
    int64_t maxSize;
    double startTime;
    double endTime;
    int64_t currentPage = 0;
    int64_t pageSize = 0;
    std::string orderBy;
    std::string order;
};

struct StaticOperatorListParams {
    std::string rankId;
    std::string deviceId;
    std::string modelName;
    std::string graphId;
    std::string searchName;
    int64_t minSize;
    int64_t maxSize;
    int64_t startNodeIndex;
    int64_t endNodeIndex;
    int64_t currentPage = 0;
    int64_t pageSize = 0;
    std::string orderBy;
    std::string order;
};

struct StaticOperatorGraphParams {
    std::string rankId;
    std::string modelName;
    std::string graphId;
    bool isCompare = false;
};

struct MemoryTypeRequest : public Request {
    MemoryTypeRequest() : Request(REQ_RES_MEMORY_TYPE) {};
    std::string rankId;
};

struct MemoryResourceTypeRequest : public Request {
    MemoryResourceTypeRequest() : Request(REQ_RES_MEMORY_RESOURCE_TYPE) {};
    std::string rankId;
};

struct MemoryOperatorRequest : public Request {
    MemoryOperatorRequest() : Request(REQ_RES_MEMORY_OPERATOR) {};
    MemoryOperatorParams params;
};

const std::string MEMORY_OVERALL_GROUP = "Overall";
const std::string MEMORY_STREAM_GROUP = "Stream";

struct MemoryComponentParams {
    std::string rankId;
    std::string type; // Overall, Stream
    bool isCompare = false;
};

struct MemoryViewRequest : public Request {
    MemoryViewRequest() : Request(REQ_RES_MEMORY_VIEW) {};
    MemoryComponentParams params;
};

struct MemoryOperatorSizeRequest : public Request {
    MemoryOperatorSizeRequest() : Request(REQ_RES_MEMORY_OPERATOR_MIN_MAX) {};
    MemoryComponentParams params;
};

struct MemoryStaticOperatorGraphRequest : public Request {
    MemoryStaticOperatorGraphRequest() : Request(REQ_RES_MEMORY_STATIC_OP_MEMORY_GRAPH) {};
    StaticOperatorGraphParams params;
};

struct MemoryStaticOperatorListRequest : public Request {
    MemoryStaticOperatorListRequest() : Request(REQ_RES_MEMORY_STATIC_OP_MEMORY_LIST) {};
    StaticOperatorListParams params;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPROTOCOLREQUEST_H
