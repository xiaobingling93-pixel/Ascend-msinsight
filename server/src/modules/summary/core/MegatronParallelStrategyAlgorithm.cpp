/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <unordered_map>
#include "ServerLog.h"
#include "MegatronParallelStrategyAlgorithm.h"

namespace Dic::Module::Summary {

const std::unordered_map<std::string, std::string> MegatronParallelStrategyAlgorithm::tokenExceptEp = {
    {DP_GROUP, DP_GROUP}, {CP_GROUP, CP_GROUP}, {TP_GROUP, TP_GROUP}, {PP_GROUP, PP_GROUP},
    {DP_CP_GROUP, DP_CP_GROUP}, {MP_GROUP, MP_GROUP_NAME}, {TP_DP_CP_GROUP, TP_DP_CP_GROUP},
    {TP_DP_GROUP, TP_DP_GROUP}, {TP_CP_GROUP, TP_CP_GROUP}};
const std::unordered_map<std::string, std::string> MegatronParallelStrategyAlgorithm::tokenWithEp = {
    {EP_GROUP, EP_GROUP_NAME}, {TP_EP_GROUP, TP_EP_GROUP_NAME}, {DP_MODULO_EP_GROUP, DP_MODULO_EP_GROUP_NAME},
    {DP_CP_MODULO_EP_GROUP, DP_CP_MODULO_EP_GROUP_NAME}, {MP_EP_GROUP, MP_EP_GROUP_NAME}};

MegatronParallelStrategyAlgorithm::MegatronParallelStrategyAlgorithm()
{
    commInfoHandlers[DIMENSIONS_TP] =
        std::bind(&MegatronParallelStrategyAlgorithm::ReduceCommTpDimensionDef, this, std::placeholders::_1);
    commInfoHandlers[DIMENSIONS_CP] =
        std::bind(&MegatronParallelStrategyAlgorithm::ReduceCommCpDimensionDef, this, std::placeholders::_1);
    commInfoHandlers[DIMENSIONS_PP] =
        std::bind(&MegatronParallelStrategyAlgorithm::ReduceCommPpDimensionDef, this, std::placeholders::_1);
}

MegatronParallelStrategyAlgorithm::~MegatronParallelStrategyAlgorithm() = default;

void MegatronParallelStrategyAlgorithm::UpdateOrderAndParallelSize()
{
    updatedOrder = paraOrder;
    updatedOrder.erase(std::remove_if(updatedOrder.begin(), updatedOrder.end(), [this](const std::string& group) {
        return !(paraDetailsMap[group].isShown);
        }), updatedOrder.end());
    parallelSize.clear();
    for (const auto& para : updatedOrder) {
        parallelSize.push_back(paraDetailsMap[para].size);
    }
    if (!paraDetailsMap[EP_PARA].isShown) {
        return;
    }
    // 仅影响连线生成，不影响布局
    updatedOrderWithEp = paraOrderWithEp;
    updatedOrderWithEp.erase(std::remove_if(updatedOrderWithEp.begin(), updatedOrderWithEp.end(),
        [this](const std::string& group) { return !(paraDetailsMap[group].isShown); }),
        updatedOrderWithEp.end());
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

bool MegatronParallelStrategyAlgorithm::UpdateParallelDimension(const std::string &tmpDimension,
    const ParallelStrategyConfig &tmpConfig, std::string &err)
{
    CalStrategyConfig(tmpDimension, tmpConfig);
    if (tmpConfig.algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG) {
        paraOrder = {TP_PARA, CP_PARA, DP_PARA, PP_PARA};
    } else if (tmpConfig.algorithm == MEGATRON_LM_TP_CP_PP_EP_DP_ALG) {
        paraOrder = {TP_PARA, CP_PARA, PP_PARA, DP_PARA};
    } else {
        err = "Failed to update parallel view. Unexpected algorithm for Megatron-LM.";
        return false;
    }
    paraOrderWithEp = paraOrder;
    if (paraOrderWithEp.back() == PP_PARA) {
        paraOrderWithEp.insert(paraOrderWithEp.begin() + epPosPpLast, EP_PARA);
    } else {
        paraOrderWithEp.insert(paraOrderWithEp.begin() + epPosDpLast, EP_PARA);
    }

    bool res = UpdateShowMap(err);
    if (res) {
        // 根据 paraDetailsMap[para].isShown 删除不存在的通信域
        UpdateOrderAndParallelSize();
        // 计算当前元素总数
        UpdateElementSize();
    }
    return res;
}

bool MegatronParallelStrategyAlgorithm::GenerateArrangementByDimension(std::string &err)
{
    ClearArrangementData();
    SetIndicatorAttr();
    std::unordered_map<std::string, uint32_t> indexAttributes;
    for (const auto& para : paraOrderWithEp) {
        indexAttributes[para + STR_INDEX] = 0;
    }
    // get arrangements
    for (uint32_t index = 0; index < elementSize; index++) {
        GetPerArrangement(index, indexAttributes);
    }
    // get connections
    if (dimension != DIMENSIONS_DP && !GetConnectionsByTokenList(err)) {
        return false;
    }
    return true;
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
    indexAttributes[EP_INDEX] = indexAttributes[DP_INDEX] / epScale;
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

    for (uint32_t ppIndex = ppIndexMin; ppIndex < ppIndexMax; ++ppIndex) {
        for (uint32_t cpIndex = cpIndexMin; cpIndex < cpIndexMax; ++cpIndex) {
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
 * 根据Token list生成全量通信域连线
 * need UpdateParallelDimension first
 * @return
 */
std::vector<Connection> MegatronParallelStrategyAlgorithm::GetAllCommunicationGroups(std::string &err)
{
    if (allCommunicationGroups.empty() && !GetConnectionsByTokenList(err)) {
        return {};
    }
    return allCommunicationGroups;
}

bool MegatronParallelStrategyAlgorithm::GetConnectionsByMegatronToken(std::string &err, bool independentEp = false)
{
    std::unordered_map<std::string, std::string> tmpToken = independentEp ? tokenWithEp : tokenExceptEp;
    for (const auto& [token, groupName] : tmpToken) {
        bool hasTokenGroup = true;
        std::vector<std::string> parallelGroups = StringUtil::Split(token, "-");
        for (const auto& group : parallelGroups) {
            if (!paraDetailsMap[group].isShown) {
                // 不存在当前含有当前并行域的通信组
                hasTokenGroup = false;
                break;
            }
        }
        if (!hasTokenGroup) {
            continue;
        }
        allGroupsType ranks{};
        if (independentEp) {
            // 计算并行通信域时需考虑ep
            ranks = ParallelStrategyAlgorithmHelper::GetAllGroupsRanksByToken(parallelGroups, parallelSizeWithEp,
                                                                              updatedOrderWithEp, wordSize);
        } else {
            // 计算并行通信域时无需考虑ep
            ranks = ParallelStrategyAlgorithmHelper::GetAllGroupsRanksByToken(parallelGroups, parallelSize,
                                                                              updatedOrder, wordSize);
        }
        if (ranks.empty()) {
            err = "Failed to get connections by token list for Megatron. Group name: " + groupName;
            return false;
        }
        for (const auto& rank : ranks) {
            data.connections.emplace_back(groupName, rank, std::vector<std::string>{});
        }
    }
    return true;
}

bool MegatronParallelStrategyAlgorithm::GetConnectionsByTokenList(std::string &err)
{
    if (wordSize == 1) {
        err = "Failed to get connections for Megatron. Parallel strategy configs have not been updated yet.";
        return false;
    }
    // 计算并行通信域时无需考虑ep
    if (!GetConnectionsByMegatronToken(err)) {
        return false;
    }
    // 计算并行通信域时需考虑ep
    if (paraDetailsMap[EP_PARA].isShown && !GetConnectionsByMegatronToken(err, true)) {
        return false;
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

void MegatronParallelStrategyAlgorithm::CalAdviceInfo(const std::string &dimension, std::vector<std::string> &advices,
                                                      std::vector<IndicatorDataStruct> &indicatorData)
{
    BaseParallelStrategyAlgorithm::CalAdviceInfo(dimension, advices, indicatorData);
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

bool MegatronParallelStrategyAlgorithm::GetPerformanceIndicatorByDimension(
    const GetPerformanceIndicatorParam &performanceParams,
    const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
    std::vector<IndicatorDataStruct> &indicatorData,
    std::string& err)
{
    if (!(strategyConfig==performanceParams.config)) {
        err = "Failed to get parallelism performance indicator for Megatron-LM. Unexpected parallel config.";
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
    if (strategyConfig.algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG) {
        ReducePpPerformanceForPpLast();
    } else {
        ReducePpPerformanceForDpLast();
    }
    if (performanceParams.dimension == DIMENSIONS_DP) {
        GetPerformanceResponseDataWithDpDimension(reducePpStatistic, indicatorData);
        return true;
    }
    err = "Failed to get parallelism performance indicator for Megatron-LM. Unexpected dimension.";
    return false;
}
}