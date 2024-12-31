/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <cfloat>
#include <unordered_map>
#include "ServerLog.h"
#include "NumberUtil.h"
#include "MegatronParallelStrategyAlgorithm.h"

namespace Dic::Module {
MegatronParallelStrategyAlgorithm::MegatronParallelStrategyAlgorithm() = default;

MegatronParallelStrategyAlgorithm::~MegatronParallelStrategyAlgorithm() = default;

void MegatronParallelStrategyAlgorithm::ClearStrategyConfigCache()
{
    // arrangements data
    data.size = 0;
    data.indicators.clear();
    data.arrangements.clear();
    data.connections.clear();
    elementSize = 1;
    paraOrder.clear();
    paraDetailsMap.clear();

    // performance data
    wordSize = 1;
    tpSize = 1;
    tpCpSize = 1;
    tpCpDpSize = 1;
    tpCpPpSize = 1;
    reduceTpStatistic.clear();
    reducePpStatistic.clear();
    reduceCpStatistic.clear();
}

bool MegatronParallelStrategyAlgorithm::UpdateParallelDimension(const std::string &tmpDimension,
    const ParallelStrategyConfig &tmpConfig, std::string &err)
{
    strategyConfig = tmpConfig;
    dimension = tmpDimension;
    tpSize = tmpConfig.tpSize;
    tpCpSize = tpSize * tmpConfig.cpSize;
    tpCpDpSize = tpCpSize * tmpConfig.dpSize;
    tpCpPpSize = tpCpSize * tmpConfig.ppSize;
    wordSize = tpCpSize * tmpConfig.dpSize * tmpConfig.ppSize;
    if (tmpConfig.algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG || tmpConfig.algorithm == MEGATRON_LM_TP_DP_PP_ALG) {
        paraOrder = {TP_PARA, CP_PARA, DP_PARA, PP_PARA};
    } else if (tmpConfig.algorithm == MEGATRON_LM_TP_CP_PP_EP_DP_ALG ||
               tmpConfig.algorithm == MEGATRON_LM_TP_PP_DP_ALG) {
        paraOrder = {TP_PARA, CP_PARA, PP_PARA, DP_PARA};
    } else {
        err = "Failed to update parallel view. Unexpected algorithm.";
        return false;
    }
    bool res = UpdateShowMap(err);
    UpdateElementSize();
    return res;
}

void MegatronParallelStrategyAlgorithm::UpdateElementSize()
{
    elementSize = 1;
    // 前端入参已校验，无整数溢出风险
    for (const auto& para : paraOrder) {
        elementSize *= paraDetailsMap[para].size;
    }
    data.size = elementSize;
}

bool MegatronParallelStrategyAlgorithm::UpdateShowMap(std::string &err)
{
    // 按层次关闭showMap, 被关闭的层次size置1，接下来就不用感知区别不同层次size的影响了
    for (const auto& para : paraOrder) {
        paraDetailsMap[para].isShown = false;
        paraDetailsMap[para].size = 1;
    }
    SetParaDetail(EP_PARA, strategyConfig.epSize);
    SetParaDetail(DP_PARA, strategyConfig.dpSize);
    if (dimension == DIMENSIONS_DP) {
        return true;
    }
    SetParaDetail(CP_PARA, strategyConfig.cpSize);
    if (dimension == DIMENSIONS_CP) {
        return true;
    }
    SetParaDetail(PP_PARA, strategyConfig.ppSize);
    if (dimension == DIMENSIONS_PP) {
        return true;
    }
    SetParaDetail(TP_PARA, strategyConfig.tpSize);
    if (dimension == DIMENSIONS_TP) {
        return true;
    } else {
        err = "Failed to update show map for parallel view. Unexpected dimension.";
    }
    return false;
}

void MegatronParallelStrategyAlgorithm::SetParaDetail(const std::string &para, int64_t size)
{
    paraDetailsMap[para].isShown = true;
    paraDetailsMap[para].size = size;
}

void MegatronParallelStrategyAlgorithm::GenerateArrangementByDimension()
{
    ClearArrangementData();
    SetIndicatorAttr();
    std::unordered_map<std::string, uint32_t> indexAttributes;
    for (const auto& para : paraOrder) {
        indexAttributes[para + STR_INDEX] = 0;
    }
    // get arrangements
    for (uint32_t index = 0; index < elementSize; index++) {
        GetPerArrangement(index, indexAttributes);
    }
    // get connections
    for (auto& element : data.arrangements) {
        GetConnections(element);
    }
}

void MegatronParallelStrategyAlgorithm::ClearArrangementData()
{
    data.indicators.clear();
    data.arrangements.clear();
    data.connections.clear();
}

ArrangementAndConnectionData MegatronParallelStrategyAlgorithm::GetArrangementData()
{
    return data;
}

void MegatronParallelStrategyAlgorithm::SetTpIndicatorAttr()
{
    // 总计算、总通信
    data.indicators.emplace_back(KEY_TOTAL_COMPUTING_TIME, VALUE_TOTAL_COMPUTING_TIME, true, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_TOTAL_COMMUNICATION, VALUE_TOTAL_COMMUNICATION, true, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    // Communication(Not Overlapped and Exclude Receive)
    data.indicators.emplace_back(KEY_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE,
        VALUE_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE, true, false, false, BAR_CHART, "", TIME_AXIS);

    // 参与stack堆叠：预处理、纯计算、重叠、纯通信、下发
    data.indicators.emplace_back(KEY_PREPARING_TIME, VALUE_PREPARING_TIME, true, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(KEY_PURE_COMPUTING_TIME, VALUE_PURE_COMPUTING_TIME, false, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(KEY_COMMUNICATION_OVERLAPPED, VALUE_COMMUNICATION_OVERLAPPED, true, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(KEY_COMMUNICATION_NOT_OVERLAPPED, VALUE_COMMUNICATION_NOT_OVERLAPPED, true, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(KEY_FREE_TIME, VALUE_FREE_TIME, true, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    // stage and bubble
    data.indicators.emplace_back(KEY_STAGE_TIME, VALUE_STAGE_TIME, false, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_BUBBLE_TIME, VALUE_BUBBLE_TIME, false, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    // ratio
    data.indicators.emplace_back(KEY_COMPUTING_RATIO, VALUE_COMPUTING_RATIO, false, true, true,
                                 LINE_CHART, "", RATIO_AXIS);
    data.indicators.emplace_back(KEY_COMMUNICATION_RATIO, VALUE_COMMUNICATION_RATIO, false, true, true,
                                 LINE_CHART, "", RATIO_AXIS);
}

void MegatronParallelStrategyAlgorithm::SetPpIndicatorAttr()
{
    // 总计算、总通信, 默认显示总计算
    data.indicators.emplace_back(KEY_TOTAL_COMPUTING_TIME, VALUE_MAX + VALUE_TOTAL_COMPUTING_TIME, true, true, true,
                                 BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_TOTAL_COMMUNICATION, VALUE_MAX + VALUE_TOTAL_COMMUNICATION, true, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    // 通信掩盖、通信未掩盖、下发
    data.indicators.emplace_back(KEY_COMMUNICATION_OVERLAPPED, VALUE_MAX + VALUE_COMMUNICATION_OVERLAPPED,
        true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_COMMUNICATION_NOT_OVERLAPPED, VALUE_MAX + VALUE_COMMUNICATION_NOT_OVERLAPPED,
                                 true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_FREE_TIME, VALUE_MAX + VALUE_FREE_TIME, true, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    // npu总时间
    data.indicators.emplace_back(KEY_NPU_TIME, VALUE_MAX + VALUE_NPU_TIME, true, true, false,
                                 BAR_CHART, "", TIME_AXIS);
}

void MegatronParallelStrategyAlgorithm::SetCpIndicatorAttr()
{
    // 总计算、总通信, 默认显示总计算
    data.indicators.emplace_back(KEY_TOTAL_COMPUTING_TIME, VALUE_SUM_OF_MAX + VALUE_TOTAL_COMPUTING_TIME,
                                 true, true, true, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_TOTAL_COMMUNICATION, VALUE_SUM_OF_MAX + VALUE_TOTAL_COMMUNICATION,
                                 true, true, false, BAR_CHART, "", TIME_AXIS);
    // 通信掩盖、通信未掩盖、下发
    data.indicators.emplace_back(KEY_COMMUNICATION_OVERLAPPED, VALUE_SUM_OF_MAX + VALUE_COMMUNICATION_OVERLAPPED,
                                 true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_COMMUNICATION_NOT_OVERLAPPED,
        VALUE_SUM_OF_MAX + VALUE_COMMUNICATION_NOT_OVERLAPPED, true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_FREE_TIME, VALUE_SUM_OF_MAX + VALUE_FREE_TIME,
        true, true, false, BAR_CHART, "", TIME_AXIS);
}

void MegatronParallelStrategyAlgorithm::SetDpIndicatorAttr()
{
    // 总计算、总通信, 默认显示总计算
    data.indicators.emplace_back(KEY_TOTAL_COMPUTING_TIME, VALUE_TOTAL_COMPUTING_TIME, true, true, true,
                                 BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_TOTAL_COMMUNICATION, VALUE_TOTAL_COMMUNICATION, true, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    // 通信掩盖、通信未掩盖、下发
    data.indicators.emplace_back(KEY_COMMUNICATION_OVERLAPPED, VALUE_COMMUNICATION_OVERLAPPED,
                                 true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_COMMUNICATION_NOT_OVERLAPPED,
                                 VALUE_COMMUNICATION_NOT_OVERLAPPED, true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_FREE_TIME, VALUE_FREE_TIME,
                                 true, true, false, BAR_CHART, "", TIME_AXIS);
}

void MegatronParallelStrategyAlgorithm::SetIndicatorAttr()
{
    if (dimension == DIMENSIONS_TP) {
        SetTpIndicatorAttr();
    } else if (dimension == DIMENSIONS_PP) {
        SetPpIndicatorAttr();
    } else if (dimension == DIMENSIONS_CP) {
        SetCpIndicatorAttr();
    } else {
        SetDpIndicatorAttr();
    }
}

void MegatronParallelStrategyAlgorithm::GetPerArrangement(uint32_t index,
    std::unordered_map<std::string, uint32_t> &indexAttributes)
{
    Element element;
    element.index = index;
    if (index != 0) {
        UpdateIndexAttributes(indexAttributes);
    } else {
        indexAttributes[EP_PARA + STR_INDEX] = 0;
    }
    element.indexAttributes = indexAttributes;
    element.name = GetElementName(indexAttributes);
    element.position = GetElementPosition(indexAttributes);
    element.ranks = GetElementContainRanks(index, indexAttributes);
    data.arrangements.push_back(element);
}

void MegatronParallelStrategyAlgorithm::UpdateIndexAttributes(
    std::unordered_map<std::string, uint32_t> &indexAttributes)
{
    // 由低层次到高层次遍历各并行域，依次检查是否需要进位
    std::string curIndex;
    for (const auto& curPara : paraOrder) {
        // 未达到size-1，无需进位
        if (indexAttributes[curPara + STR_INDEX] < paraDetailsMap[curPara].size - 1) {
            indexAttributes[curPara + STR_INDEX]++;
            break;
        }
        // 达到size-1，当前位置0，检查下一位
        if (curPara != paraOrder.back()) {
            indexAttributes[curPara + STR_INDEX] = 0;
        }
    }
    // 添加epIndex, 前端入参已校验，分母不可能为零, dpSize一定能被epSize整除
    uint32_t epScale = paraDetailsMap[DP_PARA].size / paraDetailsMap[EP_PARA].size;
    indexAttributes[EP_PARA + STR_INDEX] = indexAttributes[DP_PARA + STR_INDEX] / epScale;
}

std::string MegatronParallelStrategyAlgorithm::GetElementName(
    std::unordered_map<std::string, uint32_t> &indexAttributes)
{
    std::string name;
    for (const auto& para : LAYOUT) {
        if (paraDetailsMap[para].isShown && GetParallelSizeByType(para) > 1) {
            name += para;
            name += std::to_string(indexAttributes[para + STR_INDEX]);
            name += "-";
        }
    }
    if (!name.empty()) {
        name.pop_back();
    }
    return name;
}

Position MegatronParallelStrategyAlgorithm::GetElementPosition(
    std::unordered_map<std::string, uint32_t>& indexAttributes)
{
    Position position;
    // 前端入参已校验，无整数溢出风险
    uint32_t dpLen = paraDetailsMap[TP_PARA].size * paraDetailsMap[CP_PARA].size;
    uint32_t cpLen = paraDetailsMap[TP_PARA].size;
    position.x = indexAttributes[DP_PARA + STR_INDEX] * dpLen + indexAttributes[CP_PARA + STR_INDEX] * cpLen
        + indexAttributes[TP_PARA + STR_INDEX];
    position.y = indexAttributes[PP_PARA + STR_INDEX];
    return position;
}

std::vector<uint32_t> MegatronParallelStrategyAlgorithm::GetElementContainRanks(uint32_t index,
    std::unordered_map<std::string, uint32_t> &attrs)
{
    std::vector<uint32_t> ranks{};
    if (wordSize <= 1) {
        return ranks;
    }
    if (dimension == DIMENSIONS_TP) {
        ranks.emplace_back(index);
        return ranks;
    }

    uint32_t ppIndexMin = 0;
    uint32_t ppIndexMax = strategyConfig.ppSize;
    uint32_t cpIndexMin = 0;
    uint32_t cpIndexMax = strategyConfig.cpSize;

    if (dimension == DIMENSIONS_PP) {
        ppIndexMin = attrs[PP_INDEX];
        ppIndexMax = ppIndexMin + 1;
        cpIndexMin = attrs[CP_INDEX];
        cpIndexMax = cpIndexMin + 1;
    } else if (dimension == DIMENSIONS_CP) {
        cpIndexMin = attrs[CP_INDEX];
        cpIndexMax = cpIndexMin + 1;
    }

    for (uint32_t cpIndex = cpIndexMin; cpIndex < cpIndexMax; ++cpIndex) {
        for (uint32_t ppIndex = ppIndexMin; ppIndex < ppIndexMax; ++ppIndex) {
            for (uint32_t tpIndex = 0; tpIndex < strategyConfig.tpSize; ++tpIndex) {
                uint32_t rank = strategyConfig.algorithm == MEGATRON_LM_TP_CP_PP_EP_DP_ALG ?
                    tpCpPpSize * attrs[DP_INDEX] + tpCpSize * ppIndex + tpSize * cpIndex + tpIndex :
                    tpCpDpSize * ppIndex + tpCpSize * attrs[DP_INDEX] + tpSize * cpIndex + tpIndex;
                ranks.emplace_back(rank);
            }
        }
    }
    return ranks;
}

void MegatronParallelStrategyAlgorithm::GetConnections(Element &curEle)
{
    // 仅支持 DIMENSIONS_TP 与 DIMENSIONS_PP 层级
    if (dimension == DIMENSIONS_DP || dimension == DIMENSIONS_CP) {
        return;
    }
    uint32_t tpLen = paraDetailsMap[TP_PARA].size;
    uint32_t cpLen = tpLen * paraDetailsMap[CP_PARA].size;
    // 求tp连接
    AddConnection(data.connections, TP_PARA, tpLen, 1, curEle);
    // 求cp连接
    AddConnection(data.connections, CP_PARA, cpLen, tpLen, curEle);
    if (strategyConfig.algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG) {
        uint32_t dpLen = cpLen * paraDetailsMap[DP_PARA].size;
        // 求dp连接
        AddConnection(data.connections, DP_PARA, dpLen, cpLen, curEle);
        // 求pp连接
        AddConnection(data.connections, PP_PARA, data.size - curEle.index, dpLen, curEle);
        return;
    } else {
        uint32_t ppLen = cpLen * paraDetailsMap[PP_PARA].size;
        // 求pp连接
        AddConnection(data.connections, PP_PARA, ppLen, cpLen, curEle);
        // 求dp连接
        AddConnection(data.connections, DP_PARA, data.size - curEle.index, ppLen, curEle);
    }
}

void MegatronParallelStrategyAlgorithm::AddConnection(std::vector<Connection> &connections,
    const std::string &paraType, uint32_t len, uint32_t stepSize, Element &curEle)
{
    if (curEle.indexAttributes[paraType + STR_INDEX] != 0) {
        return;
    }
    std::vector<uint32_t> indexes{};
    for (uint32_t index = curEle.index; index < curEle.index + len; index += stepSize) {
        indexes.push_back(index);
    }
    // 若只含单个点，则无连线，不加入connections
    if (indexes.size() > 1) {
        connections.emplace_back(paraType, indexes, std::vector<std::string>{});
    }
}

void MegatronParallelStrategyAlgorithm::SortPerformanceDataByIndex(std::vector<IndicatorDataStruct>& performanceData)
{
    std::sort(performanceData.begin(), performanceData.end(),
              [](const IndicatorDataStruct& a, const IndicatorDataStruct& b) {
                  return a.index < b.index; // 按 index 升序
              });
}

void MegatronParallelStrategyAlgorithm::AnalyzePerformanceAdviceWithDpCpPpTpDimension(Protocol::TraceStatistic &max,
    Protocol::TraceStatistic &min, double meanE2ETime, std::vector<std::string> &advices)
{
    constexpr double threshold = 0.05;
    Protocol::TraceStatistic diff = {
        max.computeDiff - min.computeDiff,
        max.communicationDiff - min.communicationDiff,
        max.freeDiff - min.freeDiff
    };
    if (diff.computeDiff / meanE2ETime > threshold) {
        advices.emplace_back("Computing has some issues, because the max difference of \"Total Computing\" "
                             "has reached " + std::to_string(diff.computeDiff) + "us.");
    }
    if (diff.communicationDiff / meanE2ETime > threshold) {
        advices.emplace_back("Communication has some issues, because the max difference of "
            "\"Communication(Not Overlapped)\" has reached " + std::to_string(diff.communicationDiff) + "us.");
    }
    if (diff.freeDiff / meanE2ETime > threshold) {
        advices.emplace_back("Free has some issues, because the max difference of \"Free\" "
                             "has reached " + std::to_string(diff.freeDiff) + "us.");
    }
}

void MegatronParallelStrategyAlgorithm::CalculatePerformanceDataWithDpCpPpTpDimension(
    const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
    PerformanceIndicatorData &performanceResponseData)
{
    Protocol::TraceStatistic max{};
    Protocol::TraceStatistic min = {DBL_MAX, DBL_MAX, DBL_MAX};
    double sum = 0;
    for (uint32_t i = 0; i < wordSize; ++i) {
        if (statistic.find(i) == statistic.end()) {
            continue;
        }
        IndicatorDataStruct one{};
        one.index = i;
        const StepStatistic &item = statistic.at(i);
        one.indicators.emplace(KEY_PREPARING_TIME,
            NumberUtil::DoubleReservedNDigits(item.prepareTime, numTwo));
        one.indicators.emplace(KEY_TOTAL_COMPUTING_TIME,
            NumberUtil::DoubleReservedNDigits(item.computingTime, numTwo));
        one.indicators.emplace(KEY_PURE_COMPUTING_TIME,
            NumberUtil::DoubleReservedNDigits(item.computingTime - item.overlapCommunicationTime, numTwo));
        one.indicators.emplace(KEY_TOTAL_COMMUNICATION,
            NumberUtil::DoubleReservedNDigits(item.communicationTime, numTwo));
        one.indicators.emplace(KEY_COMMUNICATION_OVERLAPPED,
            NumberUtil::DoubleReservedNDigits(item.overlapCommunicationTime, numTwo));
        one.indicators.emplace(KEY_COMMUNICATION_NOT_OVERLAPPED,
            NumberUtil::DoubleReservedNDigits(item.pureCommunicationTime, numTwo));
        one.indicators.emplace(KEY_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE,
            NumberUtil::DoubleReservedNDigits(item.pureCommunicationExcludeReceiveTime, numTwo));
        one.indicators.emplace(KEY_FREE_TIME, NumberUtil::DoubleReservedNDigits(item.freeTime, numTwo));
        one.indicators.emplace(KEY_STAGE_TIME, NumberUtil::DoubleReservedNDigits(item.stageTime, numTwo));
        one.indicators.emplace(KEY_BUBBLE_TIME, NumberUtil::DoubleReservedNDigits(item.bubbleTime, numTwo));
        double e2eTime = item.computingTime + item.pureCommunicationTime + item.freeTime;
        one.indicators.emplace(KEY_COMPUTING_RATIO, e2eTime == 0 ? 0 :
            NumberUtil::DoubleReservedNDigits(item.computingTime / e2eTime, 2)); // 保留2位小数
        one.indicators.emplace(KEY_COMMUNICATION_RATIO, e2eTime == 0 ? 0 :
            NumberUtil::DoubleReservedNDigits(item.communicationTime / e2eTime, 2)); // 保留2位小数
        performanceResponseData.performanceData.emplace_back(one);
        max.computeDiff = std::max(max.computeDiff, item.computingTime);
        max.communicationDiff = std::max(max.communicationDiff, item.pureCommunicationTime);
        max.freeDiff = std::max(max.freeDiff, item.freeTime);
        min.computeDiff = std::min(min.computeDiff, item.computingTime);
        min.communicationDiff = std::min(min.communicationDiff, item.pureCommunicationTime);
        min.freeDiff = std::min(min.freeDiff, item.freeTime);
        sum += e2eTime;
    }
    SortPerformanceDataByIndex(performanceResponseData.performanceData);

    if (!performanceResponseData.performanceData.empty() && sum != 0) {
        AnalyzePerformanceAdviceWithDpCpPpTpDimension(max, min,
            sum / performanceResponseData.performanceData.size(), performanceResponseData.advices);
    }
}

void MegatronParallelStrategyAlgorithm::ReduceTpPerformance(
    const std::unordered_map<std::uint32_t, StepStatistic> &statistic, uint32_t tpSize)
{
    uint32_t idx = 0;
    for (uint32_t i = 0; i < wordSize; i += tpSize) {
        StepStatistic reduceTpOne;
        // 若已经计算过，则不用重复计算
        if (reduceTpStatistic.find(idx) != reduceTpStatistic.end()) {
            idx++;
            continue;
        }
        for (uint32_t j = i; j < i + tpSize && j < wordSize; j++) {
            // 跳过空数据卡
            if (statistic.find(i) == statistic.end()) {
                continue;
            }
            const StepStatistic &item = statistic.at(i);
            reduceTpOne.computingTime = std::max(reduceTpOne.computingTime, item.computingTime);
            reduceTpOne.communicationTime = std::max(reduceTpOne.communicationTime, item.communicationTime);
            reduceTpOne.pureCommunicationTime = std::max(reduceTpOne.pureCommunicationTime, item.pureCommunicationTime);
            reduceTpOne.overlapCommunicationTime = std::max(reduceTpOne.overlapCommunicationTime,
                                                            item.overlapCommunicationTime);
            reduceTpOne.freeTime = std::max(reduceTpOne.freeTime, item.freeTime);
            reduceTpOne.npuTotalTime = std::max(reduceTpOne.npuTotalTime, item.npuTotalTime);
        }
        // 若一组TP域不全为空，则存入reduceTpStatistic
        if (reduceTpOne.computingTime != 0.0) {
            reduceTpStatistic[idx] = reduceTpOne;
        }
        idx++;
    }
}

void MegatronParallelStrategyAlgorithm::GetPerformanceResponseDataWithCollapsedDimension(
    const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
    PerformanceIndicatorData &performanceResponseData)
{
    for (const auto& item : statistic) {
        IndicatorDataStruct one{};
        one.index = item.first;
        StepStatistic indicator = item.second;
        one.indicators.emplace(KEY_TOTAL_COMPUTING_TIME,
                               NumberUtil::DoubleReservedNDigits(indicator.computingTime, numTwo));
        one.indicators.emplace(KEY_TOTAL_COMMUNICATION,
                               NumberUtil::DoubleReservedNDigits(indicator.communicationTime, numTwo));
        one.indicators.emplace(KEY_COMMUNICATION_OVERLAPPED,
                               NumberUtil::DoubleReservedNDigits(indicator.overlapCommunicationTime, numTwo));
        one.indicators.emplace(KEY_COMMUNICATION_NOT_OVERLAPPED,
                               NumberUtil::DoubleReservedNDigits(indicator.pureCommunicationTime, numTwo));
        one.indicators.emplace(KEY_FREE_TIME,
                               NumberUtil::DoubleReservedNDigits(indicator.freeTime, numTwo));
        if (indicator.npuTotalTime != 0) {
            one.indicators.emplace(KEY_NPU_TIME, NumberUtil::DoubleReservedNDigits(indicator.npuTotalTime, numTwo));
        }
        performanceResponseData.performanceData.emplace_back(one);
    }
    SortPerformanceDataByIndex(performanceResponseData.performanceData);
}

void MegatronParallelStrategyAlgorithm::ReducePpPerformanceForPpLast()
{
    uint32_t cpGroupIdx = 0;
    uint32_t cpDpWordSize = strategyConfig.cpSize * strategyConfig.dpSize;
    for (uint32_t i = 0; i < cpDpWordSize; i++) {
        ReducePpPerformance(i, cpDpWordSize, cpGroupIdx);
    }
}

void MegatronParallelStrategyAlgorithm::ReducePpPerformanceForDpLast()
{
    uint32_t cpGroupIdx = 0;
    uint32_t  cpPpWordSize = strategyConfig.cpSize * strategyConfig.ppSize;
    for (uint32_t i = 0; i < wordSize; i += cpPpWordSize) {
        for (uint32_t j = i; j < i + strategyConfig.cpSize && j < wordSize; j++) {
            ReducePpPerformance(j, strategyConfig.cpSize, cpGroupIdx);
        }
    }
}

void MegatronParallelStrategyAlgorithm::ReducePpPerformance(uint32_t startIndex, uint32_t step, uint32_t& cpGroupIdx)
{
    StepStatistic reducePpOne;
    // 若已经计算过，则不用重复计算
    if (reducePpStatistic.find(cpGroupIdx) != reducePpStatistic.end()) {
        cpGroupIdx++;
        return;
    }
    // 取累加和
    for (uint32_t k = startIndex; k < wordSize && k < startIndex + step * strategyConfig.ppSize; k += step) {
        // 跳过空数据卡
        if (reduceTpStatistic.find(k) == reduceTpStatistic.end()) {
            continue;
        }
        const StepStatistic &item = reduceTpStatistic.at(k);
        reducePpOne.computingTime += item.computingTime;
        reducePpOne.communicationTime += item.communicationTime;
        reducePpOne.pureCommunicationTime += item.pureCommunicationTime;
        reducePpOne.overlapCommunicationTime += item.overlapCommunicationTime;
        reducePpOne.freeTime += item.freeTime;
    }
    // 若一组PP域不全为空，则存入reducePpStatistic
    if (reducePpOne.computingTime != 0.0) {
        reducePpStatistic[cpGroupIdx] = reducePpOne;
    }
    cpGroupIdx++;
}

void MegatronParallelStrategyAlgorithm::ReduceCpPerformance()
{
    uint32_t idx = 0;
    for (uint32_t i = 0; i < wordSize; i += strategyConfig.cpSize) {
        StepStatistic reduceCpOne;
        // 若已经计算过，则不用重复计算
        if (reduceCpStatistic.find(idx) != reduceCpStatistic.end()) {
            idx++;
            continue;
        }
        for (uint32_t j = i; j < i + strategyConfig.cpSize && j < wordSize; j++) {
            // 跳过空数据卡
            if (reducePpStatistic.find(i) == reducePpStatistic.end()) {
                continue;
            }
            const StepStatistic &item = reducePpStatistic.at(i);
            reduceCpOne.computingTime = std::max(reduceCpOne.computingTime, item.computingTime);
            reduceCpOne.communicationTime = std::max(reduceCpOne.communicationTime, item.communicationTime);
            reduceCpOne.pureCommunicationTime = std::max(reduceCpOne.pureCommunicationTime, item.pureCommunicationTime);
            reduceCpOne.overlapCommunicationTime = std::max(reduceCpOne.overlapCommunicationTime,
                                                            item.overlapCommunicationTime);
            reduceCpOne.freeTime = std::max(reduceCpOne.freeTime, item.freeTime);
        }
        // 若一组CP域不全为空，则存入reduceCpStatistic
        if (reduceCpOne.computingTime != 0.0) {
            reduceCpStatistic[idx] = reduceCpOne;
        }
        idx++;
    }
}

bool MegatronParallelStrategyAlgorithm::GetPerformanceIndicatorByDimension(
    const Protocol::ParallelismPerformance &performanceParams,
    const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
    PerformanceIndicatorData &performanceResponseData,
    std::string& err)
{
    if (!(strategyConfig==performanceParams.config)) {
        err = "Failed to get parallelism performance indicator by dimension. Unexpected parallel config.";
        return false;
    }
    wordSize = strategyConfig.tpSize * strategyConfig.ppSize * strategyConfig.cpSize * strategyConfig.dpSize;
    if (performanceParams.dimension == DIMENSIONS_TP) {
        CalculatePerformanceDataWithDpCpPpTpDimension(statistic, performanceResponseData);
        return true;
    }
    // 折叠TP
    ReduceTpPerformance(statistic, strategyConfig.tpSize);
    if (performanceParams.dimension == DIMENSIONS_PP) {
        GetPerformanceResponseDataWithCollapsedDimension(reduceTpStatistic, performanceResponseData);
        return true;
    }
    // 折叠PP
    if (strategyConfig.algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG ||
        strategyConfig.algorithm == MEGATRON_LM_TP_DP_PP_ALG) {
        ReducePpPerformanceForPpLast();
    } else {
        ReducePpPerformanceForDpLast();
    }
    if (performanceParams.dimension == DIMENSIONS_CP) {
        GetPerformanceResponseDataWithCollapsedDimension(reducePpStatistic, performanceResponseData);
        return true;
    }
    // 折叠CP
    ReduceCpPerformance();
    if (performanceParams.dimension == DIMENSIONS_DP) {
        GetPerformanceResponseDataWithCollapsedDimension(reduceCpStatistic, performanceResponseData);
        return true;
    } else {
        err = "Failed to get parallelism performance indicator by dimension. Unexpected dimension.";
        return false;
    }
}
}