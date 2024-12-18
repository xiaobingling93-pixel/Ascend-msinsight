/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <unordered_map>
#include "ServerLog.h"
#include "MegatronParallelStrategyAlgorithm.h"

namespace Dic::Module {
MegatronParallelStrategyAlgorithm::MegatronParallelStrategyAlgorithm() = default;

MegatronParallelStrategyAlgorithm::~MegatronParallelStrategyAlgorithm() = default;

bool MegatronParallelStrategyAlgorithm::UpdateParallelDimension(const std::string &tmpDimension,
    const ParallelStrategyConfig &tmpConfig, std::string &err)
{
    config = tmpConfig;
    dimension = tmpDimension;
    if (tmpConfig.algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG) {
        paraOrder = {TP_PARA, CP_PARA, DP_PARA, PP_PARA};
    } else if (tmpConfig.algorithm == MEGATRON_LM_TP_CP_PP_EP_DP_ALG) {
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
    SetParaDetail(DP_PARA, config.dpSize);
    if (dimension == DIMENSIONS_DP) {
        return true;
    }
    SetParaDetail(CP_PARA, config.cpSize);
    if (dimension == DIMENSIONS_CP) {
        return true;
    }
    SetParaDetail(PP_PARA, config.ppSize);
    if (dimension == DIMENSIONS_PP) {
        return true;
    }
    SetParaDetail(TP_PARA, config.tpSize);
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

void MegatronParallelStrategyAlgorithm::SetIndicatorAttr()
{
    // 待整改，不同维度可能展示指标不一样
    data.indicators.emplace_back(KEY_PREPARING_TIME, VALUE_PREPARING_TIME, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(KEY_TOTAL_COMPUTING_TIME, VALUE_TOTAL_COMPUTING_TIME, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_PURE_COMPUTING_TIME, VALUE_PURE_COMPUTING_TIME, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(KEY_COMMUNICATION_OVERLAPPED, VALUE_COMMUNICATION_OVERLAPPED, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(KEY_COMMUNICATION_NOT_OVERLAPPED, VALUE_COMMUNICATION_NOT_OVERLAPPED, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(KEY_FREE_TIME, VALUE_FREE_TIME, true, true,
                                 BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(KEY_STAGE_TIME, VALUE_STAGE_TIME, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_BUBBLE_TIME, VALUE_BUBBLE_TIME, true, false,
                                 BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(KEY_COMPUTING_RATIO, VALUE_COMPUTING_RATIO, false, true,
                                 LINE_CHART, "", RATIO_AXIS);
    data.indicators.emplace_back(KEY_COMMUNICATION_RATIO, VALUE_COMMUNICATION_RATIO, false, true,
                                 LINE_CHART, "", RATIO_AXIS);
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
        if (paraDetailsMap[para].isShown) {
            name += para;
            name += std::to_string(indexAttributes[para + STR_INDEX]);
            name += "-";
        }
    }
    name.pop_back();
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
    if (config.algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG) {
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
}