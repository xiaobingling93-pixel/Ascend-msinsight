/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTER_DEF_H
#define PROFILER_SERVER_CLUSTER_DEF_H

#include <string>
#include <optional>
#include <fstream>
#include <unordered_map>
#include "ServerLog.h"

namespace Dic {
namespace Module {
struct CommunicationTimeInfo {
    std::string iterationId;
    std::string stageId;
    std::string rankId;
    std::string opName;
    std::string opSuffix;
    uint64_t startTime = 0;
    double elapseTime = 0;
    double synchronizationTimeRatio = 0;
    double synchronizationTime = 0;
    double transitTime = 0;
    double waitTimeRatio = 0;
    double waitTime = 0;
    double idleTime = 0;
};
struct CommunicationBandWidth {
    std::string iterationId;
    std::string rankId;
    std::string stageId;
    std::string opName;
    std::string opSuffix;
    std::string transportType;
    double bandwidthSize = 0;
    double bandwidthUtilization = 0;
    double largePackageRatio = 0;
    std::string sizeDistribution;
    double transitSize = 0;
    double transitTime = 0;
};
struct StepStatistic {
    std::string rankId;
    std::string stepId;
    std::string stageId;
    double computingTime = 0;
    double pureCommunicationTime = 0;
    double overlapCommunicationTime = 0;
    double communicationTime = 0;
    double freeTime = 0;
    double stageTime = 0;
    double bubbleTime = 0;
    double pureCommunicationExcludeReceiveTime = 0;
    double prepareTime = -1;
    double npuTotalTime = 0;
    int64_t dpIndex = 0;
    int64_t ppIndex = 0;
    int64_t tpIndex = 0;
};

const std::string PARALLEL_CONFIG_LEVEL_COLLECTED = "collected";
const std::string PARALLEL_CONFIG_LEVEL_PREDICTED = "predicted";
const std::string PARALLEL_CONFIG_LEVEL_CONFIRMED = "confirmed";
const std::string PARALLEL_CONFIG_LEVEL_CONFIGURED = "configured";
const std::string PARALLEL_CONFIG_LEVEL_UNDEFINED = "undefined";

const std::string MEGATRON_ALG = "megatron";
const std::string MEGATRON_LM_TP_DP_PP_ALG = "Megatron-LM(tp-dp-pp)";
const std::string MEGATRON_LM_TP_PP_DP_ALG = "Megatron-LM(tp-pp-dp)";
const std::string MEGATRON_LM_TP_CP_EP_DP_PP_ALG = "megatron-lm(tp-cp-ep-dp-pp)";
const std::string MEGATRON_LM_TP_CP_PP_EP_DP_ALG = "megatron-lm(tp-cp-pp-ep-dp)";
const std::string DIMENSIONS_DP = "ep-dp";
const std::string DIMENSIONS_PP = "ep-dp-pp";
const std::string DIMENSIONS_CP = "ep-dp-pp-cp";
const std::string DIMENSIONS_TP = "ep-dp-pp-cp-tp";
const std::string PP_PARA = "pp";
const std::string CP_PARA = "cp";
const std::string DP_PARA = "dp";
const std::string TP_PARA = "tp";
const std::string EP_PARA = "ep";
const std::string STR_INDEX = "Index";
const std::string PP_INDEX = "ppIndex";
const std::string CP_INDEX = "cpIndex";
const std::string DP_INDEX = "dpIndex";
const std::string TP_INDEX = "tpIndex";
const std::string EP_INDEX = "epIndex";
const int64_t MAX_PARALLEL_SIZE = 255;
const int64_t MAX_PARALLEL_PRODUCT_SIZE = 250000;

struct ParallelStrategyConfig {
    std::string algorithm = MEGATRON_LM_TP_DP_PP_ALG; // megatron-lm tp-dp-pp, megatron-lm tp-pp-dp
    int64_t ppSize{};
    int64_t tpSize{};
    int64_t dpSize{};
    int64_t cpSize = 1;
    int64_t epSize = 1;
    bool CheckParams(std::string &errorMsg) const
    {
        // 检查ppSize, tpSize, dpSize的范围
        if (ppSize <= 0 || ppSize > MAX_PARALLEL_SIZE) {
            errorMsg = "[Summary] PP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
            return false;
        }
        if (tpSize <= 0 || tpSize > MAX_PARALLEL_SIZE) {
            errorMsg = "[Summary] TP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
            return false;
        }
        if (dpSize <= 0 || dpSize > MAX_PARALLEL_SIZE) {
            errorMsg = "[Summary] DP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
            return false;
        }
        if (cpSize <= 0 || cpSize > MAX_PARALLEL_SIZE) {
            errorMsg = "[Summary] CP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
            return false;
        }
        if (epSize <= 0 || epSize > MAX_PARALLEL_SIZE) {
            errorMsg = "[Summary] EP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
            return false;
        }
        // 检查dpSize是否能被epSize整除
        if (dpSize % epSize != 0) {
            errorMsg = "[Summary] DP size must be evenly divided by EP Size.";
            return false;
        }
        // 检查四个数的乘积是否小于MAX_PARALLEL_PRODUCT_SIZE(25万)
        if (ppSize * tpSize * dpSize * cpSize > MAX_PARALLEL_PRODUCT_SIZE) {
            errorMsg = "[Summary] The product of PP size, TP size, DP size, and CP size must be less than " +
                       std::to_string(MAX_PARALLEL_PRODUCT_SIZE);
            return false;
        }
        return true;
    }

    bool operator==(const ParallelStrategyConfig& conf) const
    {
        if (algorithm != conf.algorithm) {
            return false;
        }
        if (ppSize != conf.ppSize || tpSize != conf.tpSize || cpSize != conf.cpSize
                || dpSize != conf.dpSize || epSize != conf.epSize) {
            return false;
        }
        return true;
    }
};

struct ClusterBaseInfo {
    std::string filePath;
    std::string ranks;
    std::string steps;
    std::string ppStages;
    std::string stages;
    long long collectStartTime;
    double collectDuration;
    long long dataSize  =  0;
    ParallelStrategyConfig config;
    std::string level;
};

struct CommunicationMatrixInfo {
    std::string groupId;
    std::string iterationId;
    std::string sortOp;
    std::string opName;
    std::string groupName;
    int srcRank;
    int dstRank;
    std::string transportType;
    double transitSize;
    double transitTime;
    double bandwidth;
};

// cluster_step_trace_time.csv表头
const std::string FIELD_STEP = "Step";
const std::string FIELD_TYPE = "Type";
const std::string FIELD_INDEX = "Index";
const std::string FIELD_COMPUTING = "Computing";
const std::string FIELD_COMMUNICATION = "Communication";
const std::string FIELD_FREE = "Free";
const std::string FIELD_COMMUNICATION_NOT_OVERLAPPED = "Communication(Not Overlapped)";
const std::string FIELD_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE = "Communication(Not Overlapped and Exclude Receive)";
const std::string FIELD_OVERLAPPED = "Overlapped";
const std::string FIELD_PREPARE_TIME = "Preparing";

const std::string FIELD_STAGE = "Stage";
const std::string FIELD_BUBBLE = "Bubble";

const std::string FIELD_DP_INDEX = "DP Index";
const std::string FIELD_PP_INDEX = "PP Index";
const std::string FIELD_TP_INDEX = "TP Index";
const std::string FIELD_CP_INDEX = "CP Index";
const std::string FIELD_EP_INDEX = "EP Index";

const std::string VALUE_PREPARING_TIME = FIELD_PREPARE_TIME;
const std::string VALUE_TOTAL_COMPUTING_TIME = FIELD_COMPUTING;
const std::string VALUE_COMPUTING_NOT_OVERLAPPED = "Computing(Not Overlapped)";
const std::string VALUE_TOTAL_COMMUNICATION = FIELD_COMMUNICATION;
const std::string VALUE_COMMUNICATION_OVERLAPPED = "Computing/Communication Overlapped";
const std::string VALUE_COMMUNICATION_NOT_OVERLAPPED = FIELD_COMMUNICATION_NOT_OVERLAPPED;
const std::string VALUE_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE = FIELD_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE;
const std::string VALUE_FREE_TIME = FIELD_FREE;
const std::string VALUE_STAGE_TIME = FIELD_STAGE;
const std::string VALUE_BUBBLE_TIME = FIELD_BUBBLE;
const std::string VALUE_NPU_TIME = "Computing + Communication(Not Overlapped) + Free";

const std::vector<std::string> VALID_STEP_STATISTICS_HEADERS = {
    FIELD_STEP, FIELD_TYPE, FIELD_INDEX, FIELD_COMPUTING, FIELD_COMMUNICATION_NOT_OVERLAPPED, FIELD_OVERLAPPED,
    FIELD_COMMUNICATION, FIELD_FREE, FIELD_STAGE, FIELD_BUBBLE, FIELD_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE
};
const std::vector<std::string> PARALLEL_STRATEGY_HEADERS = {
    FIELD_DP_INDEX, FIELD_PP_INDEX, FIELD_TP_INDEX
};

struct Position {
    uint32_t x = 0;
    uint32_t y = 0;
    bool operator==(const Position& other) const
    {
        return x == other.x && y == other.y;
    }
};

struct IndicatorAttr {
    uint8_t number{}; // No.
    std::string key;
    // 性能指标名称
    std::string name;
    // 是否需要在2D排布图上按色域渲染
    bool renderHeatMap = false;
    bool renderChart = false;
    // 计算/通信概览 是否默认显示
    bool visible = false;
    // 下方绘图时图表类型，柱形图还是折线图
    std::string chart;
    // 如果是堆积柱形图，堆积分类
    std::string stack;
    // 数据类型，用于区分y轴
    std::string yAxisType;
    IndicatorAttr() = default;
    IndicatorAttr(uint8_t number, std::string key, std::string name, bool renderHeatMap, bool renderChart,
        bool visible, std::string chart, std::string stack, std::string yAxisType)
        : number(number), key(std::move(key)), name(std::move(name)), renderHeatMap(renderHeatMap),
        renderChart(renderChart), visible(visible), chart(std::move(chart)),
        stack(std::move(stack)), yAxisType(std::move(yAxisType)) {}
};

struct Connection {
    // 界面上一条连线连接多个element
    std::vector<uint32_t> indexes{};
    // dp, pp, tp, cp...
    std::string type;
    // 界面上一条连线对应多个通信域
    std::vector<std::string> communicationGroups;
    Connection() = default;
    Connection(std::string type, const std::vector<uint32_t>& indexes,
        const std::vector<std::string>& communicationGroups) : indexes(indexes), type(std::move(type)),
        communicationGroups(communicationGroups) {}
};

// 一张卡或一个分组的相关信息，包括序号、名称、位置、并行分组属性、包含的卡等信息
struct Element {
    uint32_t index; // rank or group index
    std::string name; // rank or group name
    Position position{}; // rank or group position in 2D arrangement
    std::unordered_map<std::string, uint32_t> indexAttributes{}; // {dp_index=0}
    std::vector<uint32_t> ranks;
};

struct ArrangementAndConnectionData {
    uint32_t size;
    std::vector<IndicatorAttr> indicators;
    std::vector<Element> arrangements; // rank or group arrangement and performance data
    std::vector<Connection> connections; // connection between ranks or groups
};

struct IndicatorDataStruct {
    uint32_t index; // rank or group index
    std::unordered_map<std::string, double> indicators; // performance data
};

struct GetPerformanceIndicatorParam {
    std::string step;
    std::string dimension;
    ParallelStrategyConfig config;
};

// Summary性能数据
const std::string KEY_PREPARING_TIME = "preparing";
const std::string KEY_TOTAL_COMPUTING_TIME = "computing";
const std::string KEY_PURE_COMPUTING_TIME = "pureComputing";
const std::string KEY_TOTAL_COMMUNICATION = "communication";
const std::string KEY_COMMUNICATION_OVERLAPPED = "notOverlapped";
const std::string KEY_COMMUNICATION_NOT_OVERLAPPED = "overlapped";
const std::string KEY_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE = "excludeReceive";
const std::string KEY_FREE_TIME = "free";
const std::string KEY_STAGE_TIME = "stage";
const std::string KEY_BUBBLE_TIME = "bubble";
const std::string KEY_NPU_TIME = "npuTime";
const std::string KEY_COMPUTING_RATIO = "computingRatio";
const std::string KEY_COMMUNICATION_RATIO = "communicationRatio";
const std::string KEY_MAX_SUFFIX = "Max";
const std::string KEY_MIN_SUFFIX = "Min";
const std::string KEY_RANGE_SUFFIX = "Range";

const std::string VALUE_COMPUTING_RATIO = "Computing Ratio";
const std::string VALUE_COMMUNICATION_RATIO = "Communication Ratio";
const std::string VALUE_MAX = "Max ";
const std::string VALUE_MIN = "Min ";
const std::string VALUE_RANGE = " Range";
const std::string VALUE_SUM_OF_MAX = "Sum of Max ";

// chart type
const std::string BAR_CHART = "bar";
const std::string LINE_CHART = "line";
// stack type
const std::string TIME_STACK = "time";
// y axis type
const std::string TIME_AXIS = "time";
const std::string RATIO_AXIS = "ratio";

// 当初始状态的数据库有ClusterBaseInfo表时，存储distributed_args列的信息
struct DistributedArgs {
    ParallelStrategyConfig config{MEGATRON_LM_TP_DP_PP_ALG, 1, 1, 1, 1, 1};
    int64_t worldSize = 1;
    bool sequenceParallel = false;
};
const std::vector<std::string> DISTRIBUTED_ARGS_INT_KEY{"tensor_model_parallel_size", "pipeline_model_parallel_size",
    "data_parallel_size", "context_parallel_size", "expert_model_parallel_size", "world_size"};
const std::vector<std::string> DISTRIBUTED_ARGS_BOOL_KEY{"sequence_parallel"};
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_CLUSTER_DEF_H