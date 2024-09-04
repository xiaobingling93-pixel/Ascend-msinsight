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

template<typename T>
struct CompareData {
    T baseline;
    T compare;
    T diff;
};

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

template<typename R>
struct TableDetail {
    std::string tableName;
    std::vector<std::string> size;
    std::vector<std::string> headerName;
    std::vector<R> row;
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
    TableDetail<TableRow> blockDetail;
    std::vector<std::string> advice;
};

struct DetailsBaseInfoResponse : public Response {
    DetailsBaseInfoResponse() : Response(REQ_RES_DETAILS_BASE_INFO) {}
    CompareData<DetailsBaseInfoResBody> body;
};

struct SubBlockUnitData {
    std::string blockId;
    std::string blockType;
    std::string name;
    std::string unit;
    std::string value;
    std::string originValue;
};

struct Roofline {
    std::string bw;   // 理论带宽
    std::string bwName;
    std::string computility;  // 屋顶算力
    std::string computilityName;    // 算力名称
    std::vector<std::string> point;  // 坐标
    std::string ratio;      // 百分比
};

struct RooflineGraph {
    // 标题
    std::string title;
    std::vector<Roofline> rooflines;
};

struct RooflineData {
    std::string advice;
    std::vector<RooflineGraph> multipleRooflines;
};

struct SubBlockData {
    std::vector<CompareData<SubBlockUnitData>> detailDataList;
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
    std::vector<CompareData<MemoryUnit>> memoryUnit;
    CompareData<L2Cache> l2Cache;
    std::vector<std::string> advice;
    CompareData<UtilizationRate> vector;
    CompareData<UtilizationRate> vector1;
    CompareData<UtilizationRate> cube;
};

struct MemoryTable {
    std::string blockId;
    std::string tableOpType;
    std::vector<TableDetail<CompareData<TableRow>>> tableDetail;
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

template<typename T>
struct DetailsInterCoreLoadDimension {
    T value = 0;
    int level = 0;
};

struct DetailsInterCoreLoadSubCoreDetail {
    static const uint8_t maxLevel = 10;
    std::string subCoreName;
    DetailsInterCoreLoadDimension<uint64_t> cycles;
    DetailsInterCoreLoadDimension<uint64_t> throughput;
    DetailsInterCoreLoadDimension<float> cacheHitRate;

    void SetCyclesDimension(uint64_t curCycles, uint64_t minCycles)
    {
        if (minCycles == 0 || curCycles < minCycles) { // 说明所有的cycles数据都为0
            return;
        }
        cycles.value = curCycles;
        // 比较当前cycle数和最小cycle数的差值，如果大于最小cycles数，那么level直接置为1
        uint64_t diff = curCycles - minCycles;
        if (diff >= minCycles) {
            cycles.level = 1;
            return;
        }
        // 每增加10%，level由MAX_LEVEL减少1
        cycles.level = maxLevel - diff * 10 / minCycles;
        if (cycles.level < 1) {
            cycles.level = 1;
        }
    }

    void SetThroughputDimension(uint64_t curThroughput, uint64_t minThroughput)
    {
        if (minThroughput == 0) { // 说明所有的throughput数据都为0
            return;
        }
        throughput.value = curThroughput;
        // 比较当前的throughput和最小的throughput的差值
        uint64_t diff = curThroughput - minThroughput;
        if (diff > minThroughput) {
            // 如果差值大于一倍, level直接置为1
            throughput.level = 1;
            return;
        }
        // 每增加10%，level由MAX_LEVEL减少1直到等于1
        throughput.level = maxLevel - diff * 10 / minThroughput;
        if (throughput.level < 1) {
            throughput.level = 1;
        }
    }

    void SetCacheHitRateDimension(float curRate, float maxRate)
    {
        if (maxRate == 0) { // 说明所有的cache rate hit数据都为0
            return;
        }
        cacheHitRate.value = curRate;
        // 比较当前的cache hit rate和最大的rate之间的差值，每减小10%，level由MAX_LEVEL减少1直到等于1
        cacheHitRate.level = maxLevel - (maxRate - curRate) * 10 / maxRate;
        if (cacheHitRate.level < 1) {
            cacheHitRate.level = 1;
        }
    }

    void SetSubCoreName(const std::string& type, uint8_t id)
    {
        subCoreName = type + std::to_string(id);
    }
};

struct DetailsInterCoreLoadOpDetail {
    uint8_t coreId = 0;
    std::vector<DetailsInterCoreLoadSubCoreDetail> subCoreDetails = {};

    void AddSubCoreDetail(DetailsInterCoreLoadSubCoreDetail&& subCoreDetail)
    {
        subCoreDetails.emplace_back(std::move(subCoreDetail));
    }
};

struct DetailsInterCoreLoadGraphBody {
    std::string soc;
    std::string opType;
    std::string advice;
    std::vector<DetailsInterCoreLoadOpDetail> opDetails = {};

    void AddOpDetail(DetailsInterCoreLoadOpDetail&& opDetail)
    {
        opDetails.emplace_back(std::move(opDetail));
    }
};

struct DetailsInterCoreLoadGraphResponse : public Response {
    DetailsInterCoreLoadGraphResponse() : Response(REQ_RES_DETAILS_INTER_CORE_LOAD_GRAPH) {};
    DetailsInterCoreLoadGraphBody body;
};

struct DetailsRooflineBody {
    std::string soc;
    std::string advice;
    std::vector<RooflineGraph> data;
};

struct DetailsRooflineResponse : public Response {
    DetailsRooflineResponse() : Response(std::string(REQ_RES_DETAILS_ROOFLINE)) {};
    DetailsRooflineBody body;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_SOURCE_PROTOCOL_SOURCE_RESPONSE_H
