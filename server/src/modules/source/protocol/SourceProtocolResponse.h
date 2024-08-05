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

struct TableRow {
    std::string name;
    std::vector<std::string> value;
};

struct TableDetail {
    std::string tableName;
    std::vector<std::string> size;
    std::vector<std::string> headerName;
    std::vector<TableRow> row;
};

struct DetailsBaseInfoResBody {
    std::string name;
    std::string soc;
    std::string opType;
    std::string blockDim;
    std::string mixBlockDim;
    std::string duration;
    std::string deviceId;
    std::string pid;
    TableDetail blockDetail;
    std::vector<std::string> advice;
};

struct DetailsBaseInfoResponse : public Response {
    DetailsBaseInfoResponse() : Response(REQ_RES_DETAILS_BASE_INFO) {}
    DetailsBaseInfoResBody body;
};

struct SubBlockUnitData {
    std::string blockId;
    std::string blockType;
    std::string name;
    std::string unit;
    std::string value;
    std::string originValue;
};

struct SubBlockData {
    std::vector<SubBlockUnitData> detailDataList;
    std::vector<std::string> advice;
};

struct DetailsLoadInfoResBody {
    std::vector<std::string> blockIdList;
    SubBlockData chartData;
    SubBlockData tableData;
};

struct DetailsLoadInfoResponse : public Response {
    DetailsLoadInfoResponse() : Response(REQ_RES_DETAILS_COMPUTE_LOAD_INFO) {}
    DetailsLoadInfoResBody body;
};

struct MemoryUnit {
    std::string memoryPath;
    uint64_t request;
    std::string bandwidth;
    std::string peakRatio;
    bool display;
};

struct L2Cache {
    std::string hit;
    std::string miss;
    std::string totalRequest;
    std::string hitRatio;
};

struct UtilizationRate {
    std::string cycle;
    std::string totalCycles;
    std::string ratio;
};

struct MemoryGraph {
    std::string blockId;
    std::string blockType;
    std::string chipType;
    std::vector<MemoryUnit> memoryUnit;
    L2Cache l2Cache;
    std::vector<std::string> advice;
    UtilizationRate vector;
    UtilizationRate vector1;
    UtilizationRate cube;
};

struct MemoryTable {
    std::string blockId;
    std::string tableOpType;
    std::vector<TableDetail> tableDetail;
    std::vector<std::string> advice;
};
struct DetailsMemoryGraphResBody {
    std::vector<MemoryGraph> coreMemory;
};

struct DetailsMemoryGraphResponse : public Response {
    DetailsMemoryGraphResponse() : Response(REQ_RES_DETAILS_COMPUTE_MEMORY_GRAPH) {};
    DetailsMemoryGraphResBody body;
};

struct DetailsMemoryTableResBody {
    std::vector<MemoryTable> memoryTable;
};

struct DetailsMemoryTableResponse : public Response {
    DetailsMemoryTableResponse() : Response(REQ_RES_DETAILS_COMPUTE_MEMORY_TABLE) {};
    DetailsMemoryTableResBody body;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_SOURCE_PROTOCOL_SOURCE_RESPONSE_H
