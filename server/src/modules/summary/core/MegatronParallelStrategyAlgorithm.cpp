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
    paraOrderWithEp.clear();
    paraDetailsMap.clear();
    updatedOrder.clear();
    updatedOrderWithEp.clear();
    parallelSize.clear();
    parallelSizeWithEp.clear();

    // performance data
    wordSize = 1;
    tpSize = 1;
    tpCpSize = 1;
    tpCpDpSize = 1;
    tpCpPpSize = 1;
    reduceTpMax.clear();
    reduceTpMin.clear();
    reducePpStatistic.clear();
    reduceCpMax.clear();
    reduceCpMin.clear();
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
    paraOrderWithEp = paraOrder;
    if (paraOrderWithEp.back() == PP_PARA) {
        paraOrderWithEp.insert(paraOrderWithEp.begin() + epPosPpLast, EP_PARA);
    } else {
        paraOrderWithEp.insert(paraOrderWithEp.begin() + epPosDpLast, EP_PARA);
    }

    bool res = UpdateShowMap(err);

    // 删除不存在的通信域
    updatedOrder = paraOrder;
    updatedOrder.erase(std::remove_if(updatedOrder.begin(), updatedOrder.end(), [this](const std::string& group) {
        return !(paraDetailsMap[group].isShown);
        }), updatedOrder.end());
    parallelSize.clear();
    for (const auto& para : updatedOrder) {
        parallelSize.push_back(paraDetailsMap[para].size);
    }

    if (paraDetailsMap[EP_PARA].isShown) {
        // 仅影响连线生成，不影响布局
        updatedOrderWithEp = paraOrderWithEp;
        updatedOrderWithEp.erase(std::remove_if(updatedOrderWithEp.begin(), updatedOrderWithEp.end(),
            [this](const std::string& group) { return !(paraDetailsMap[group].isShown); }), updatedOrderWithEp.end());
        parallelSizeWithEp.clear();
        for (const auto& para : updatedOrderWithEp) {
            if (para == DP_PARA) {
                // 前端入参已校验
                parallelSizeWithEp.push_back(paraDetailsMap[DP_PARA].size / paraDetailsMap[EP_PARA].size);
            } else {
                parallelSizeWithEp.push_back(paraDetailsMap[para].size);
            }
        }
    }
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
    for (const auto& para : paraOrderWithEp) {
        paraDetailsMap[para].isShown = false;
        paraDetailsMap[para].size = 1;
    }
    SetParaDetail(EP_PARA, strategyConfig.epSize);
    SetParaDetail(DP_PARA, strategyConfig.dpSize);
    if (dimension == DIMENSIONS_DP) {
        return true;
    }
    SetParaDetail(PP_PARA, strategyConfig.ppSize);
    if (dimension == DIMENSIONS_PP) {
        return true;
    }
    SetParaDetail(CP_PARA, strategyConfig.cpSize);
    if (dimension == DIMENSIONS_CP) {
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
    // 参数为1，该并行域未生效
    if (size == 1) {
        return;
    }
    paraDetailsMap[para].isShown = true;
    paraDetailsMap[para].size = size;
}

bool MegatronParallelStrategyAlgorithm::GenerateArrangementByDimension(std::string &err)
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
    if (dimension == DIMENSIONS_TP) {
        if (!GetConnectionsByTokenList(err)) {
            return false;
        }
    } else {
        // get connections for cp or pp dimension
        for (auto& element : data.arrangements) {
            GetConnections(element);
        }
    }
    return true;
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

// 对于TP View，展示全部原始数据，及step_trace_time中的内容
void MegatronParallelStrategyAlgorithm::SetTpIndicatorAttr()
{
    uint8_t index = 0;
    // 总计算、总通信不在柱状图中显示，而是通过掩盖和未掩盖的堆叠形成
    data.indicators.emplace_back(
        index++, KEY_TOTAL_COMPUTING_TIME, VALUE_TOTAL_COMPUTING_TIME, true, false, true, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(
        index++, KEY_TOTAL_COMMUNICATION, VALUE_TOTAL_COMMUNICATION, true, false, true, BAR_CHART, "", TIME_AXIS);

    // 参与stack堆叠：预处理、纯计算、重叠、纯通信、下发
    data.indicators.emplace_back(index++, KEY_PURE_COMPUTING_TIME, VALUE_COMPUTING_NOT_OVERLAPPED,
                                 true, true, true, BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_COMMUNICATION_OVERLAPPED, VALUE_COMMUNICATION_OVERLAPPED,
                                 true, true, true, BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_COMMUNICATION_NOT_OVERLAPPED, VALUE_COMMUNICATION_NOT_OVERLAPPED,
                                 true, true, true, BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(
        index++, KEY_FREE_TIME, VALUE_FREE_TIME, true, true, true, BAR_CHART, TIME_STACK, TIME_AXIS);
    data.indicators.emplace_back(
        index++, KEY_PREPARING_TIME, VALUE_PREPARING_TIME, true, true, true, BAR_CHART, TIME_STACK, TIME_AXIS);

    // Communication(Not Overlapped and Exclude Receive)
    data.indicators.emplace_back(index++, KEY_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE,
        VALUE_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE, true, false, false, BAR_CHART, "", TIME_AXIS);

    // stage and bubble
    data.indicators.emplace_back(
        index++, KEY_STAGE_TIME, VALUE_STAGE_TIME, true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(
        index++, KEY_BUBBLE_TIME, VALUE_BUBBLE_TIME, true, false, false, BAR_CHART, "", TIME_AXIS);

    // ratio
    data.indicators.emplace_back(
        index++, KEY_COMPUTING_RATIO, VALUE_COMPUTING_RATIO, false, true, true, LINE_CHART, "", RATIO_AXIS);
    data.indicators.emplace_back(
        index++, KEY_COMMUNICATION_RATIO, VALUE_COMMUNICATION_RATIO, false, true, true, LINE_CHART, "", RATIO_AXIS);
}

// 对于CP Dimension，通过展示一个TP域内计算/通信等时间的统计值，包括最大值、最小值、极差，来分析CP域的性能瓶颈
// 对于通信，可关注最小值，有时通信最小的点，才是瓶颈所在
// 通过极差，可以反映TP域内各卡计算、通信是否均衡，尤其是计算
void MegatronParallelStrategyAlgorithm::SetCpIndicatorAttr()
{
    uint8_t index = 0;
    data.indicators.emplace_back(index++, KEY_TOTAL_COMPUTING_TIME + KEY_MAX_SUFFIX,
                                 VALUE_MAX + VALUE_TOTAL_COMPUTING_TIME, true, true, true, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMMUNICATION + KEY_MAX_SUFFIX,
                                 VALUE_MAX + VALUE_TOTAL_COMMUNICATION, true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_FREE_TIME + KEY_MAX_SUFFIX, VALUE_MAX + VALUE_FREE_TIME,
                                 true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_NPU_TIME + KEY_MAX_SUFFIX, VALUE_MAX + VALUE_NPU_TIME,
                                 true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMPUTING_TIME + KEY_MIN_SUFFIX,
                                 VALUE_MIN + VALUE_TOTAL_COMPUTING_TIME, true, false, true, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMMUNICATION + KEY_MIN_SUFFIX,
                                 VALUE_MIN + VALUE_TOTAL_COMMUNICATION, true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_FREE_TIME + KEY_MIN_SUFFIX, VALUE_MIN + VALUE_FREE_TIME,
                                 true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_NPU_TIME + KEY_MIN_SUFFIX, VALUE_MIN + VALUE_NPU_TIME,
                                 true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMPUTING_TIME + KEY_RANGE_SUFFIX,
                                 VALUE_TOTAL_COMPUTING_TIME + VALUE_RANGE, true, false, true, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMMUNICATION + KEY_RANGE_SUFFIX,
                                 VALUE_TOTAL_COMMUNICATION + VALUE_RANGE, true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_FREE_TIME + KEY_RANGE_SUFFIX, VALUE_FREE_TIME + VALUE_RANGE,
                                 true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_NPU_TIME + KEY_RANGE_SUFFIX, VALUE_NPU_TIME + VALUE_RANGE,
                                 true, false, false, BAR_CHART, "", TIME_AXIS);

    // 通信掩盖、通信未掩盖
    data.indicators.emplace_back(index++, KEY_COMMUNICATION_NOT_OVERLAPPED + KEY_MAX_SUFFIX,
        VALUE_MAX + VALUE_COMMUNICATION_NOT_OVERLAPPED, true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_COMMUNICATION_NOT_OVERLAPPED + KEY_MIN_SUFFIX,
        VALUE_MIN + VALUE_COMMUNICATION_NOT_OVERLAPPED, true, false, false, BAR_CHART, "", TIME_AXIS);
}

// 对于PP Dimension，通过展示一个TP&CP域内计算/通信等时间的统计值，包括最大值、最小值、极差，来分析PP域的性能瓶颈
// 对于通信，可关注最小值，有时通信最小的点，才是瓶颈所在
// 通过极差，可以反映TP&CP域内各卡计算、通信是否均衡，尤其是计算
void MegatronParallelStrategyAlgorithm::SetPpIndicatorAttr()
{
    uint8_t index = 0;
    // 总计算、总通信、下发、npu总时间, 默认显示总计算
    data.indicators.emplace_back(index++, KEY_TOTAL_COMPUTING_TIME + KEY_MAX_SUFFIX,
                                 VALUE_MAX + VALUE_TOTAL_COMPUTING_TIME, true, true, true, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMMUNICATION + KEY_MAX_SUFFIX,
                                 VALUE_MAX + VALUE_TOTAL_COMMUNICATION, true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_FREE_TIME + KEY_MAX_SUFFIX, VALUE_MAX + VALUE_FREE_TIME,
                                 true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_NPU_TIME + KEY_MAX_SUFFIX, VALUE_MAX + VALUE_NPU_TIME,
                                 true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMPUTING_TIME + KEY_MIN_SUFFIX,
                                 VALUE_MIN + VALUE_TOTAL_COMPUTING_TIME, true, false, true, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMMUNICATION + KEY_MIN_SUFFIX,
                                 VALUE_MIN + VALUE_TOTAL_COMMUNICATION, true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_FREE_TIME + KEY_MIN_SUFFIX, VALUE_MIN + VALUE_FREE_TIME,
                                 true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_NPU_TIME + KEY_MIN_SUFFIX, VALUE_MIN + VALUE_NPU_TIME,
                                 true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMPUTING_TIME + KEY_RANGE_SUFFIX,
                                 VALUE_TOTAL_COMPUTING_TIME + VALUE_RANGE, true, false, true, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_TOTAL_COMMUNICATION + KEY_RANGE_SUFFIX,
                                 VALUE_TOTAL_COMMUNICATION + VALUE_RANGE, true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_FREE_TIME + KEY_RANGE_SUFFIX, VALUE_FREE_TIME + VALUE_RANGE,
                                 true, false, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_NPU_TIME + KEY_RANGE_SUFFIX, VALUE_NPU_TIME + VALUE_RANGE,
                                 true, false, false, BAR_CHART, "", TIME_AXIS);

    // 通信掩盖、通信未掩盖
    data.indicators.emplace_back(index++, KEY_COMMUNICATION_NOT_OVERLAPPED + KEY_MAX_SUFFIX,
        VALUE_MAX + VALUE_COMMUNICATION_NOT_OVERLAPPED, true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, KEY_COMMUNICATION_NOT_OVERLAPPED + KEY_MIN_SUFFIX,
        VALUE_MIN + VALUE_COMMUNICATION_NOT_OVERLAPPED, true, false, false, BAR_CHART, "", TIME_AXIS);
}

// 对于DP Dimension，通过展示一个TP&CP域内计算/通信等时间的统计值，包括最大值、最小值、极差，来分析PP域的性能瓶颈
void MegatronParallelStrategyAlgorithm::SetDpIndicatorAttr()
{
    uint8_t index = 0;
    // 总计算、总通信, 默认显示总计算
    data.indicators.emplace_back(index++, VALUE_SUM_OF_MAX + KEY_TOTAL_COMPUTING_TIME,
        VALUE_SUM_OF_MAX + VALUE_TOTAL_COMPUTING_TIME, true, true, true, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, VALUE_SUM_OF_MAX + KEY_TOTAL_COMMUNICATION,
        VALUE_SUM_OF_MAX + VALUE_TOTAL_COMMUNICATION, true, true, false, BAR_CHART, "", TIME_AXIS);
    data.indicators.emplace_back(index++, VALUE_SUM_OF_MAX + KEY_FREE_TIME, VALUE_SUM_OF_MAX + VALUE_FREE_TIME,
        true, true, false, BAR_CHART, "", TIME_AXIS);
    // 通信掩盖、通信未掩盖、下发
    data.indicators.emplace_back(index++, VALUE_SUM_OF_MAX + KEY_COMMUNICATION_NOT_OVERLAPPED,
        VALUE_SUM_OF_MAX + VALUE_COMMUNICATION_NOT_OVERLAPPED, true, false, false, BAR_CHART, "", TIME_AXIS);
}

void MegatronParallelStrategyAlgorithm::SetIndicatorAttr()
{
    if (dimension == DIMENSIONS_TP) {
        SetTpIndicatorAttr();
    } else if (dimension == DIMENSIONS_CP) {
        SetCpIndicatorAttr();
    } else if (dimension == DIMENSIONS_PP) {
        SetPpIndicatorAttr();
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

    if (dimension == DIMENSIONS_CP) {
        cpIndexMin = attrs[CP_INDEX];
        cpIndexMax = cpIndexMin + 1;
        ppIndexMin = attrs[PP_INDEX];
        ppIndexMax = ppIndexMin + 1;
    } else if (dimension == DIMENSIONS_PP) {
        ppIndexMin = attrs[PP_INDEX];
        ppIndexMax = ppIndexMin + 1;
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

/**
 * 根据Token list生成全量通信域连线，目前仅适配全展开视图
 * need UpdateParallelDimension first
 * @return
 */
std::vector<Connection> MegatronParallelStrategyAlgorithm::GetAllCommunicationGroups(std::string &err)
{
    if (allCommunicationGroups.empty() && GetConnectionsByTokenList(err)) {
        return {};
    }
    return allCommunicationGroups;
}

bool MegatronParallelStrategyAlgorithm::GetConnectionsByTokenList(std::string &err)
{
    if (wordSize == 1) {
        err = "Failed to get connections. Parallel strategy configs have not been updated yet.";
        return false;
    }
    for (const auto& pair : tokenNameList) {
        bool hasTokenGroup = true;
        std::string token = pair.first;
        std::vector<std::string> parallelGroups = StringUtil::Split(token, "-");
        for (const auto& group : parallelGroups) {
            if (!paraDetailsMap[group].isShown) {
                // 不存在当前含有当前并行域的通信组
                hasTokenGroup = false;
                break;
            }
        }
        allGroupsType ranks{};
        std::string groupName = pair.second;
        if (hasTokenGroup) {
            if (std::find(independentEpList.begin(), independentEpList.end(), groupName) != independentEpList.end()) {
                // 计算并行通信域时需考虑ep
                ranks = ParallelStrategyAlgorithmHelper::GetAllGroupsRanksByToken(parallelGroups, parallelSizeWithEp,
                                                                                  updatedOrderWithEp, wordSize);
            } else {
                // 计算并行通信域时无需考虑ep
                ranks = ParallelStrategyAlgorithmHelper::GetAllGroupsRanksByToken(parallelGroups, parallelSize,
                                                                                  updatedOrder, wordSize);
            }
            if (ranks.empty()) {
                err = "Failed to get connections by token list. Token: " + token;
                return false;
            }
        }
        for (const auto& rank : ranks) {
            data.connections.emplace_back(groupName, rank, std::vector<std::string>{});
        }
    }
    if (dimension == DIMENSIONS_TP) {
        allCommunicationGroups = data.connections;
    }
    return true;
}

void MegatronParallelStrategyAlgorithm::GetConnections(Element &curEle)
{
    // 全展开维度的连线由GetConnectionsByTokenList()生成
    static const std::vector<std::string> dimsHasConnection = {DIMENSIONS_CP, DIMENSIONS_PP};
    if (std::find(dimsHasConnection.begin(), dimsHasConnection.end(), dimension) == dimsHasConnection.end()) {
        return;
    }
    uint32_t hiddenSize = (elementSize == 0 || wordSize == 0) ? 1 : (wordSize / elementSize);
    // 求tp连接，范围为[0, tp size)，步长为1
    AddConnection(data.connections, TP_PARA, tpSize / hiddenSize, 1, curEle);
    // 求cp连接，范围为[0, cp_size * tp_size)，步长为tp_size
    AddConnection(data.connections, CP_PARA, tpCpSize / hiddenSize, tpSize / hiddenSize, curEle);

    if (strategyConfig.algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG) {
        // 求dp连接，范围为[0, cp_size * tp_size * dp_size)，步长为tp size * cp size
        AddConnection(data.connections, DP_PARA, tpCpDpSize / hiddenSize, tpCpSize / hiddenSize, curEle);
        // 求pp连接，范围为[0, cp size，步长为tp size * cp size * dp size
        AddConnection(data.connections, PP_PARA, data.size - curEle.index, tpCpDpSize / hiddenSize, curEle);
        return;
    } else {
        // 求pp连接，范围为cp size，步长为tp size * cp size * dp size
        AddConnection(data.connections, PP_PARA, tpCpPpSize / hiddenSize, tpCpSize / hiddenSize, curEle);
        // 求dp连接，范围为cp size，步长为tp size * cp size
        AddConnection(data.connections, DP_PARA, data.size - curEle.index, tpCpPpSize, curEle);
    }
}

void MegatronParallelStrategyAlgorithm::AddConnection(std::vector<Connection> &connections,
    const std::string &paraType, uint32_t len, uint32_t stepSize, Element &curEle)
{
    if (curEle.indexAttributes[paraType + STR_INDEX] != 0) {
        return;
    }
    if (len <= 1) {
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
        advices.emplace_back("Computing has some issues, because the max difference of \"Computing\" "
            "has reached " + std::to_string(Reserved3DecimalPlaces(diff.computeDiff)) + "us.");
    }
    if (diff.communicationDiff / meanE2ETime > threshold) {
        advices.emplace_back("Communication has some issues, because the max difference of "
            "\"Communication(Not Overlapped)\" has reached " +
            std::to_string(Reserved3DecimalPlaces(diff.communicationDiff)) + "us.");
    }
    if (diff.freeDiff / meanE2ETime > threshold) {
        advices.emplace_back("Free has some issues, because the max difference of \"Free\" "
            "has reached " + std::to_string(Reserved3DecimalPlaces(diff.freeDiff)) + "us.");
    }
}

void MegatronParallelStrategyAlgorithm::CalculatePerformanceDataWithTpDimension(
    const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
    std::vector<IndicatorDataStruct> &indicatorData)
{
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
            NumberUtil::DoubleReservedNDigits(item.computingTime / e2eTime * PERCENTAGE_RATIO_SCALE, numTwo));
        one.indicators.emplace(KEY_COMMUNICATION_RATIO, e2eTime == 0 ? 0 :
            NumberUtil::DoubleReservedNDigits(item.communicationTime / e2eTime * PERCENTAGE_RATIO_SCALE, numTwo));
        indicatorData.emplace_back(one);
    }
    SortPerformanceDataByIndex(indicatorData);
}

void MegatronParallelStrategyAlgorithm::CalAdviceInfo(const std::string &dimension, std::vector<std::string> &advices,
                                                      std::vector<IndicatorDataStruct> &indicatorData)
{
    if (dimension == DIMENSIONS_TP) {
        Protocol::TraceStatistic max{};
        Protocol::TraceStatistic min = {DBL_MAX, DBL_MAX, DBL_MAX};
        double sum = 0;
        for (auto &item: indicatorData) {
            max.computeDiff = std::max(max.computeDiff, item.indicators[KEY_TOTAL_COMPUTING_TIME]);
            max.communicationDiff = std::max(max.communicationDiff, item.indicators[KEY_COMMUNICATION_NOT_OVERLAPPED]);
            max.freeDiff = std::max(max.freeDiff, item.indicators[KEY_FREE_TIME]);
            min.computeDiff = std::min(min.computeDiff, item.indicators[KEY_TOTAL_COMPUTING_TIME]);
            min.communicationDiff = std::min(min.communicationDiff, item.indicators[KEY_COMMUNICATION_NOT_OVERLAPPED]);
            min.freeDiff = std::min(min.freeDiff, item.indicators[KEY_FREE_TIME]);
            sum += item.indicators[KEY_TOTAL_COMPUTING_TIME] + item.indicators[KEY_COMMUNICATION_NOT_OVERLAPPED] +
                item.indicators[KEY_FREE_TIME];
        }
        if (!indicatorData.empty() && sum != 0) {
            AnalyzePerformanceAdviceWithDpCpPpTpDimension(max, min, sum / indicatorData.size(), advices);
        }
    }
}

void MegatronParallelStrategyAlgorithm::ReduceTpPerformance(
    const std::unordered_map<std::uint32_t, StepStatistic> &statistic)
{
    uint32_t idx = 0;
    for (uint32_t i = 0; i < wordSize; i += tpSize) {
        // 若已经计算过，则不用重复计算
        if (reduceTpMax.find(idx) != reduceTpMax.end()) {
            idx++;
            continue;
        }
        StepStatistic maxTpOne;
        StepStatistic minTpOne = {"", "", "",
            DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, 0, 0, 0};
        for (uint32_t j = i; j < i + tpSize && j < wordSize; j++) {
            // 跳过空数据卡
            if (statistic.find(j) == statistic.end()) {
                continue;
            }
            const StepStatistic &item = statistic.at(j);
            maxTpOne.computingTime = std::max(maxTpOne.computingTime, item.computingTime);
            maxTpOne.communicationTime = std::max(maxTpOne.communicationTime, item.communicationTime);
            maxTpOne.pureCommunicationTime = std::max(maxTpOne.pureCommunicationTime, item.pureCommunicationTime);
            maxTpOne.overlapCommunicationTime = std::max(maxTpOne.overlapCommunicationTime,
                                                         item.overlapCommunicationTime);
            maxTpOne.freeTime = std::max(maxTpOne.freeTime, item.freeTime);
            maxTpOne.npuTotalTime = std::max(maxTpOne.npuTotalTime, item.npuTotalTime);

            minTpOne.computingTime = std::min(minTpOne.computingTime, item.computingTime);
            minTpOne.communicationTime = std::min(minTpOne.communicationTime, item.communicationTime);
            minTpOne.pureCommunicationTime = std::min(minTpOne.pureCommunicationTime, item.pureCommunicationTime);
            minTpOne.overlapCommunicationTime = std::min(minTpOne.overlapCommunicationTime,
                                                         item.overlapCommunicationTime);
            minTpOne.freeTime = std::min(minTpOne.freeTime, item.freeTime);
            minTpOne.npuTotalTime = std::min(minTpOne.npuTotalTime, item.npuTotalTime);
        }
        // 若一组TP域不全为空，则存入reduceTpStatistic
        if (maxTpOne.computingTime != 0.0) {
            reduceTpMax[idx] = maxTpOne;
            reduceTpMin[idx] = minTpOne;
        }
        idx++;
    }
}

// 折叠CP域，形成DP+PP视图的性能数据，根据TP折叠后的最大/小值进一步计算最大值、最小值、极差等统计值
void MegatronParallelStrategyAlgorithm::ReduceCpPerformance()
{
    uint32_t idx = 0;
    for (uint32_t i = 0; i < wordSize / strategyConfig.tpSize; i += strategyConfig.cpSize) {
        // 若已经计算过，则不用重复计算
        if (reduceCpMax.find(idx) != reduceCpMax.end()) {
            idx++;
            continue;
        }
        StepStatistic maxCpOne;
        StepStatistic minCpOne = {"", "", "",
            DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX, 0, 0, 0, 0, DBL_MAX, 0, 0, 0};
        for (uint32_t j = i; j < i + strategyConfig.cpSize && j < wordSize; j++) {
            // 跳过空数据卡，此处不判断reduceTpMin，因为它与reduceTpMax总是成对出现
            if (reduceTpMax.find(j) == reduceTpMax.end()) {
                continue;
            }
            const StepStatistic &maxItem = reduceTpMax.at(j);

            maxCpOne.computingTime = std::max(maxCpOne.computingTime, maxItem.computingTime);
            maxCpOne.communicationTime = std::max(maxCpOne.communicationTime, maxItem.communicationTime);
            maxCpOne.pureCommunicationTime = std::max(maxCpOne.pureCommunicationTime, maxItem.pureCommunicationTime);
            maxCpOne.freeTime = std::max(maxCpOne.freeTime, maxItem.freeTime);
            maxCpOne.npuTotalTime = std::max(maxCpOne.npuTotalTime, maxItem.npuTotalTime);

            const StepStatistic &minItem = reduceTpMin.at(j);
            minCpOne.computingTime = std::min(minCpOne.computingTime, minItem.computingTime);
            minCpOne.communicationTime = std::min(minCpOne.communicationTime, minItem.communicationTime);
            minCpOne.pureCommunicationTime = std::min(minCpOne.pureCommunicationTime, minItem.pureCommunicationTime);
            minCpOne.freeTime = std::min(minCpOne.freeTime, minItem.freeTime);
            minCpOne.npuTotalTime = std::min(minCpOne.npuTotalTime, minItem.npuTotalTime);
        }
        // 若一组CP域不全为空，则存入
        if (maxCpOne.computingTime != 0.0) {
            reduceCpMax[idx] = maxCpOne;
            reduceCpMin[idx] = minCpOne;
        }
        idx++;
    }
}

void MegatronParallelStrategyAlgorithm::CalculatePerformanceDataWithCpDimension(
    std::vector<IndicatorDataStruct> &indicatorData)
{
    // 前面的逻辑保证tpSize不为0
    for (uint32_t i = 0; i < wordSize / tpSize; ++i) {
        if (reduceTpMax.find(i) == reduceTpMax.end()) {
            continue;
        }
        IndicatorDataStruct one{};
        one.index = i;
        auto &max = reduceTpMax.at(i);
        auto &min = reduceTpMin.at(i);
        one.indicators.emplace(KEY_TOTAL_COMPUTING_TIME + KEY_MAX_SUFFIX, max.computingTime);
        one.indicators.emplace(KEY_TOTAL_COMPUTING_TIME + KEY_MIN_SUFFIX, min.computingTime);
        one.indicators.emplace(KEY_TOTAL_COMPUTING_TIME + KEY_RANGE_SUFFIX,
                               Reserved3DecimalPlaces(max.computingTime - min.computingTime));
        one.indicators.emplace(KEY_TOTAL_COMMUNICATION + KEY_MAX_SUFFIX, max.communicationTime);
        one.indicators.emplace(KEY_TOTAL_COMMUNICATION + KEY_MIN_SUFFIX, min.communicationTime);
        one.indicators.emplace(KEY_TOTAL_COMMUNICATION + KEY_RANGE_SUFFIX,
                               Reserved3DecimalPlaces(max.communicationTime - min.communicationTime));
        one.indicators.emplace(KEY_FREE_TIME + KEY_MAX_SUFFIX, max.freeTime);
        one.indicators.emplace(KEY_FREE_TIME + KEY_MIN_SUFFIX, min.freeTime);
        one.indicators.emplace(KEY_FREE_TIME + KEY_RANGE_SUFFIX,
                               Reserved3DecimalPlaces(max.freeTime - min.freeTime));
        one.indicators.emplace(KEY_NPU_TIME + KEY_MAX_SUFFIX, max.npuTotalTime);
        one.indicators.emplace(KEY_NPU_TIME + KEY_MIN_SUFFIX, min.npuTotalTime);
        one.indicators.emplace(KEY_NPU_TIME + KEY_RANGE_SUFFIX,
                               Reserved3DecimalPlaces(max.npuTotalTime - min.npuTotalTime));
        one.indicators.emplace(KEY_COMMUNICATION_NOT_OVERLAPPED + KEY_MAX_SUFFIX, max.pureCommunicationTime);
        one.indicators.emplace(KEY_COMMUNICATION_NOT_OVERLAPPED + KEY_MIN_SUFFIX, min.pureCommunicationTime);
        indicatorData.emplace_back(one);
    }
    SortPerformanceDataByIndex(indicatorData);
}

void MegatronParallelStrategyAlgorithm::CalculatePerformanceDataWithPpDimension(
    std::vector<IndicatorDataStruct> &indicatorData)
{
    // 前面的逻辑保证tpCpSize不为0
    for (uint32_t i = 0; i < wordSize / tpCpSize; ++i) {
        if (reduceCpMax.find(i) == reduceCpMax.end()) {
            continue;
        }
        IndicatorDataStruct one{};
        one.index = i;
        auto &max = reduceCpMax.at(i);
        auto &min = reduceCpMin.at(i);
        one.indicators.emplace(KEY_TOTAL_COMPUTING_TIME + KEY_MAX_SUFFIX,
                               Reserved3DecimalPlaces(max.computingTime));
        one.indicators.emplace(KEY_TOTAL_COMPUTING_TIME + KEY_MIN_SUFFIX,
                               Reserved3DecimalPlaces(min.computingTime));
        one.indicators.emplace(KEY_TOTAL_COMPUTING_TIME + KEY_RANGE_SUFFIX,
                               Reserved3DecimalPlaces(max.computingTime - min.computingTime));
        one.indicators.emplace(KEY_TOTAL_COMMUNICATION + KEY_MAX_SUFFIX,
                               Reserved3DecimalPlaces(max.communicationTime));
        one.indicators.emplace(KEY_TOTAL_COMMUNICATION + KEY_MIN_SUFFIX,
                               Reserved3DecimalPlaces(min.communicationTime));
        one.indicators.emplace(KEY_TOTAL_COMMUNICATION + KEY_RANGE_SUFFIX,
                               Reserved3DecimalPlaces(max.communicationTime - min.communicationTime));
        one.indicators.emplace(KEY_FREE_TIME + KEY_MAX_SUFFIX,
                               Reserved3DecimalPlaces(max.freeTime));
        one.indicators.emplace(KEY_FREE_TIME + KEY_MIN_SUFFIX, Reserved3DecimalPlaces(min.freeTime));
        one.indicators.emplace(KEY_FREE_TIME + KEY_RANGE_SUFFIX,
                               Reserved3DecimalPlaces(max.freeTime - min.freeTime));
        one.indicators.emplace(KEY_NPU_TIME + KEY_MAX_SUFFIX, Reserved3DecimalPlaces(max.npuTotalTime));
        one.indicators.emplace(KEY_NPU_TIME + KEY_MIN_SUFFIX, Reserved3DecimalPlaces(min.npuTotalTime));
        one.indicators.emplace(KEY_NPU_TIME + KEY_RANGE_SUFFIX,
                               Reserved3DecimalPlaces(max.npuTotalTime - min.npuTotalTime));
        one.indicators.emplace(KEY_COMMUNICATION_NOT_OVERLAPPED + KEY_MAX_SUFFIX,
                               Reserved3DecimalPlaces(max.pureCommunicationTime));
        one.indicators.emplace(KEY_COMMUNICATION_NOT_OVERLAPPED + KEY_MIN_SUFFIX,
                               Reserved3DecimalPlaces(min.pureCommunicationTime));
        indicatorData.emplace_back(one);
    }
    SortPerformanceDataByIndex(indicatorData);
}

void MegatronParallelStrategyAlgorithm::GetPerformanceResponseDataWithDpDimension(
    const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
    std::vector<IndicatorDataStruct> &indicatorData)
{
    for (const auto& item : statistic) {
        IndicatorDataStruct one{};
        one.index = item.first;
        StepStatistic indicator = item.second;
        one.indicators.emplace(VALUE_SUM_OF_MAX + KEY_TOTAL_COMPUTING_TIME,
                               Reserved3DecimalPlaces(indicator.computingTime));
        one.indicators.emplace(VALUE_SUM_OF_MAX + KEY_TOTAL_COMMUNICATION,
                               Reserved3DecimalPlaces(indicator.communicationTime));
        one.indicators.emplace(VALUE_SUM_OF_MAX + KEY_COMMUNICATION_NOT_OVERLAPPED,
                               Reserved3DecimalPlaces(indicator.pureCommunicationTime));
        one.indicators.emplace(VALUE_SUM_OF_MAX + KEY_FREE_TIME, Reserved3DecimalPlaces(indicator.freeTime));
        indicatorData.emplace_back(one);
    }
    SortPerformanceDataByIndex(indicatorData);
}

void MegatronParallelStrategyAlgorithm::ReducePpPerformanceForPpLast()
{
    uint32_t dpGroupIdx = 0;
    // 规约后共有dp_size个数据，且规约时步长为dp_size
    for (uint32_t i = 0; i < strategyConfig.dpSize; i++) {
        ReducePpPerformance(i, strategyConfig.dpSize, dpGroupIdx);
    }
}

void MegatronParallelStrategyAlgorithm::ReducePpPerformanceForDpLast()
{
    uint32_t dpGroupIdx = 0;
    uint32_t  cpPpWordSize = strategyConfig.cpSize * strategyConfig.ppSize;
    for (uint32_t i = 0; i < wordSize; i += cpPpWordSize) {
        for (uint32_t j = i; j < i + strategyConfig.dpSize && j < wordSize; j++) {
            ReducePpPerformance(j, strategyConfig.dpSize, dpGroupIdx);
        }
    }
}

void MegatronParallelStrategyAlgorithm::ReducePpPerformance(uint32_t startIndex, uint32_t step, uint32_t& dpGroupIdx)
{
    // 若已经计算过，则不用重复计算
    if (reducePpStatistic.find(dpGroupIdx) != reducePpStatistic.end()) {
        dpGroupIdx++;
        return;
    }
    StepStatistic reducePpOne;
    // 取累加和
    for (uint32_t k = startIndex; k < wordSize / tpCpSize && k < startIndex + step * strategyConfig.ppSize; k += step) {
        // 跳过空数据卡
        if (reduceCpMax.find(k) == reduceCpMax.end()) {
            continue;
        }
        const StepStatistic &item = reduceCpMax.at(k);
        reducePpOne.computingTime += item.computingTime;
        reducePpOne.communicationTime += item.communicationTime;
        reducePpOne.pureCommunicationTime += item.pureCommunicationTime;
        reducePpOne.freeTime += item.freeTime;
    }
    // 若一组PP域不全为空，则存入reducePpStatistic
    if (reducePpOne.computingTime != 0.0) {
        reducePpStatistic[dpGroupIdx] = reducePpOne;
    }
    dpGroupIdx++;
}

bool MegatronParallelStrategyAlgorithm::GetPerformanceIndicatorByDimension(
    const GetPerformanceIndicatorParam &performanceParams,
    const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
    std::vector<IndicatorDataStruct> &indicatorData,
    std::string& err)
{
    if (!(strategyConfig==performanceParams.config)) {
        err = "Failed to get parallelism performance indicator by dimension. Unexpected parallel config.";
        return false;
    }
    tpSize = strategyConfig.tpSize;
    wordSize = strategyConfig.tpSize * strategyConfig.ppSize * strategyConfig.cpSize * strategyConfig.dpSize;
    if (performanceParams.dimension == DIMENSIONS_TP) {
        CalculatePerformanceDataWithTpDimension(statistic, indicatorData);
        return true;
    }
    // 折叠TP
    ReduceTpPerformance(statistic);
    if (performanceParams.dimension == DIMENSIONS_CP) {
        // DP+PP+CP视图时，折叠TP，计算最大值、最小值、极差等统计值
        CalculatePerformanceDataWithCpDimension(indicatorData);
        return true;
    }

    // 折叠CP
    ReduceCpPerformance();
    if (performanceParams.dimension == DIMENSIONS_PP) {
        CalculatePerformanceDataWithPpDimension(indicatorData);
        return true;
    }
    // 折叠PP
    if (strategyConfig.algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG ||
        strategyConfig.algorithm == MEGATRON_LM_TP_DP_PP_ALG) {
        ReducePpPerformanceForPpLast();
    } else {
        ReducePpPerformanceForDpLast();
    }
    if (performanceParams.dimension == DIMENSIONS_DP) {
        GetPerformanceResponseDataWithDpDimension(reducePpStatistic, indicatorData);
        return true;
    } else {
        err = "Failed to get parallelism performance indicator by dimension. Unexpected dimension.";
        return false;
    }
}

double MegatronParallelStrategyAlgorithm::Reserved3DecimalPlaces(double num)
{
    if (num == 0.0 || num == DBL_MAX) {
        return 0.0;
    }
    const int placeNum = 3;
    return NumberUtil::DoubleReservedNDigits(num, placeNum);
}
}