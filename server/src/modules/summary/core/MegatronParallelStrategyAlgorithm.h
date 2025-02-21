/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H

#include <unordered_map>
#include "SummaryProtocolResponse.h"
#include "BaseParallelStrategyAlgorithm.h"
#include "ParallelStrategyAlgorithmHelper.h"

namespace Dic::Module {

struct ParallelDetails {
    bool isShown = false;  // shownMap 代表当前层次是否展示
    uint32_t size = 1; // sizeMap ppSize等。若当前层次已折叠，size置1
};

// Token list
const std::string DP_GROUP = "dp";
const std::string CP_GROUP = "cp";
const std::string TP_GROUP = "tp";
const std::string PP_GROUP = "pp";
const std::string DP_CP_GROUP = "dp-cp";
const std::string MP_GROUP = "tp-pp";
const std::string MP_GROUP_NAME = "mp";
const std::string TP_DP_CP_GROUP = "tp-dp-cp";
const std::string TP_DP_GROUP = "tp-dp";
const std::string TP_CP_GROUP = "tp-cp";
const std::string EP_GROUP = "ep";
const std::string EP_GROUP_NAME = "exp";
const std::string TP_EP_GROUP = "tp-ep";
const std::string TP_EP_GROUP_NAME = "tp_exp";
const std::string DP_MODULO_EP_GROUP = "dp";
const std::string DP_MODULO_EP_GROUP_NAME = "dp_modulo_exp";
const std::string DP_CP_MODULO_EP_GROUP = "dp-cp";
const std::string DP_CP_MODULO_EP_GROUP_NAME = "dp_modulo_exp_cp";
const std::string MP_EP_GROUP = "tp-ep-pp";
const std::string MP_EP_GROUP_NAME = "mp_exp";
const std::unordered_map<std::string, std::string> tokenNameList = {
    {DP_GROUP, DP_GROUP}, {CP_GROUP, CP_GROUP}, {TP_GROUP, TP_GROUP}, {PP_GROUP, PP_GROUP}, {DP_CP_GROUP, DP_CP_GROUP},
    {MP_GROUP, MP_GROUP_NAME}, {TP_DP_CP_GROUP, TP_DP_CP_GROUP}, {TP_DP_GROUP, TP_DP_GROUP}, {TP_CP_GROUP, TP_CP_GROUP},
    {EP_GROUP, EP_GROUP_NAME}, {TP_EP_GROUP, TP_EP_GROUP_NAME}, {DP_MODULO_EP_GROUP, DP_MODULO_EP_GROUP_NAME},
    {DP_CP_MODULO_EP_GROUP, DP_CP_MODULO_EP_GROUP_NAME}, {MP_EP_GROUP, MP_EP_GROUP_NAME}
};
const std::vector<std::string> independentEpList = {EP_GROUP_NAME, TP_EP_GROUP_NAME, DP_MODULO_EP_GROUP_NAME,
    DP_CP_MODULO_EP_GROUP_NAME, MP_EP_GROUP_NAME};

class MegatronParallelStrategyAlgorithm : public BaseParallelStrategyAlgorithm {
public:
    MegatronParallelStrategyAlgorithm();
    ~MegatronParallelStrategyAlgorithm() override;

    void ClearStrategyConfigCache() override;
    bool UpdateParallelDimension(const std::string &tmpDimension,
                                 const ParallelStrategyConfig &tmpConfig, std::string &err) override;

    bool GenerateArrangementByDimension(std::string &err) override;
    ArrangementAndConnectionData GetArrangementData() override;
    bool GetPerformanceIndicatorByDimension(const GetPerformanceIndicatorParam &performanceParams,
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        std::vector<IndicatorDataStruct> &indicatorData, std::string& err) override;
    void CalAdviceInfo(const std::string &dimension, std::vector<std::string> &advices,
                       std::vector<IndicatorDataStruct> &indicatorData) override;
    // get all communication groups
    std::vector<Connection> GetAllCommunicationGroups(std::string &err) override;
private:
    // get arrangements
    void ClearArrangementData();
    void UpdateElementSize();
    bool UpdateShowMap(std::string &err);
    void SetParaDetail(const std::string &para, int64_t size);
    void SetTpIndicatorAttr();
    void SetPpIndicatorAttr();
    void SetCpIndicatorAttr();
    void SetDpIndicatorAttr();
    void SetIndicatorAttr();
    void GetPerArrangement(uint32_t index, std::unordered_map<std::string, uint32_t> &indexAttributes);
    void UpdateIndexAttributes(std::unordered_map<std::string, uint32_t> &indexAttributes);
    std::string GetElementName(std::unordered_map<std::string, uint32_t> &indexAttributes);
    Position GetElementPosition(std::unordered_map<std::string, uint32_t>& indexAttributes);
    std::vector<uint32_t> GetElementContainRanks(uint32_t index,
                                                 std::unordered_map<std::string, uint32_t> &indexAttributes);
    // get connections
    bool GetConnectionsByTokenList(std::string &err);
    void GetConnections(Element &curEle);
    void AddConnection(std::vector<Connection> &connections, const std::string &paraType, uint32_t len,
                       uint32_t stepSize, Element &curEle);
    // get performance data
    void SortPerformanceDataByIndex(std::vector<IndicatorDataStruct>& performanceData);
    void CalculatePerformanceDataWithTpDimension(
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        std::vector<IndicatorDataStruct> &performanceResponseData);
    void CalculatePerformanceDataWithCpDimension(std::vector<IndicatorDataStruct> &indicatorData);
    void CalculatePerformanceDataWithPpDimension(std::vector<IndicatorDataStruct> &indicatorData);
    void ReduceTpPerformance(const std::unordered_map<std::uint32_t, StepStatistic> &statistic);
    void GetPerformanceResponseDataWithDpDimension(
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        std::vector<IndicatorDataStruct> &indicatorData);
    void ReducePpPerformanceForPpLast();
    void ReducePpPerformanceForDpLast();
    void ReducePpPerformance(uint32_t startIndex, uint32_t step, uint32_t& dpGroupIdx);
    void ReduceCpPerformance();
    double Reserved3DecimalPlaces(double num);

    ArrangementAndConnectionData data;
    uint32_t elementSize = 1;
    std::string dimension = DIMENSIONS_DP; // 默认一层级
    std::vector<std::string> paraOrder;
    std::vector<std::string> paraOrderWithEp;
    std::vector<std::string> updatedOrder;
    std::vector<std::string> updatedOrderWithEp;
    std::vector<uint32_t> parallelSize;
    std::vector<uint32_t> parallelSizeWithEp; // 仅影响连线生成，不影响布局
    std::vector<Connection> allCommunicationGroups; // 全量通信域
    std::unordered_map<std::string, ParallelDetails> paraDetailsMap; // 记录某并行域是否被折叠

    // get performance data
    uint32_t wordSize = 1;
    uint32_t tpSize = 1;
    uint32_t tpCpSize = 1;
    uint32_t tpCpDpSize = 1;
    uint32_t tpCpPpSize = 1;
    std::unordered_map<std::uint32_t, StepStatistic> reduceTpMax;
    std::unordered_map<std::uint32_t, StepStatistic> reduceTpMin;
    std::unordered_map<std::uint32_t, StepStatistic> reduceCpMax;
    std::unordered_map<std::uint32_t, StepStatistic> reduceCpMin;
    std::unordered_map<std::uint32_t, StepStatistic> reducePpStatistic;
    const static inline int numTwo = 2; // 保留2位小数
    const static inline int epPosPpLast = 2; // tp-cp-ep-dp-pp
    const static inline int epPosDpLast = 3; // tp-cp-pp-ep-dp

    void AnalyzePerformanceAdviceWithDpCpPpTpDimension(Protocol::TraceStatistic &max, Protocol::TraceStatistic &min,
                                                       double meanE2ETime, std::vector<std::string> &advices);
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
