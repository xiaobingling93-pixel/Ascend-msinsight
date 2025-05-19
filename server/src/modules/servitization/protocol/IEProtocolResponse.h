/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IEPROTOCOLRESPONSE_H
#define PROFILER_SERVER_IEPROTOCOLRESPONSE_H
#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
namespace Dic::Protocol {
struct IEUsageViewData {
    std::string title;
    std::vector<std::string> legends;
    std::vector<std::vector<std::string>> lines;
};
struct IEUsageViewResponse : public Response {
    IEUsageViewResponse() : Response(REQ_RES_IE_VIEW) {}
    IEUsageViewData data;
};

struct Column {
    std::string name;
    std::string type;
    std::string key;
};

struct IETableViewData {
    std::vector<Column> columnAttr;
    std::vector<std::map<std::string, std::string>> columnData;
    uint64_t totalNum = 0;
};
struct IETableViewResponse : public Response {
    IETableViewResponse() : Response(REQ_RES_IE_TABLE_VIEW) {}
    IETableViewData data;
};

struct IEGroupData {
    std::string label;
    std::string value;
};
struct IEGroupResponse : public Response {
    IEGroupResponse() : Response(REQ_RES_IE_DATA_GROUP) {}
    std::vector<IEGroupData> data;
};
}

#endif // PROFILER_SERVER_IEPROTOCOLRESPONSE_H
