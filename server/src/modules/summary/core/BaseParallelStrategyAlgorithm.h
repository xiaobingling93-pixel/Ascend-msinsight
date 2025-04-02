/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H

#include <unordered_map>
#include <functional>
#include "ClusterDef.h"
#include "SummaryProtocolResponse.h"
#include "SummaryProtocolRequest.h"

namespace Dic::Module::Summary {

struct ParallelDetails {
    bool isShown = false;  // shownMap 代表当前层次是否展示
    uint32_t size = 1; // sizeMap ppSize等。若当前层次已折叠，size置1
};

const std::vector<std::string> LAYOUT = {DP_PARA, PP_PARA, CP_PARA, TP_PARA};

class BaseParallelStrategyAlgorithm {
public:
    BaseParallelStrategyAlgorithm() = default;
    virtual ~BaseParallelStrategyAlgorithm() = default;

    virtual void SetStrategyConfig(const ParallelStrategyConfig& config);
    ParallelStrategyConfig GetStrategyConfig();
    void ClearStrategyConfigCache();
    virtual bool UpdateParallelDimension(const std::string &dimension,
                                         const ParallelStrategyConfig &tmpConfig, std::string &err) = 0;
    virtual bool GenerateArrangementByDimension(std::string &err) = 0;
    ArrangementAndConnectionData GetArrangementData();
    virtual bool GetPerformanceIndicatorByDimension(const GetPerformanceIndicatorParam &params,
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        std::vector<IndicatorDataStruct> &indicatorData, std::string &err) = 0;
    virtual void CalAdviceInfo(const std::string &dimension, std::vector<std::string> &advices,
                               std::vector<IndicatorDataStruct> &indicatorData);
    virtual std::vector<Connection> GetAllCommunicationGroups(std::string &err) = 0;
    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> GetCommInfoByDimension(
        const std::unordered_map<std::string, std::vector<CommInfoUnderRank>> &expandCommInfos,
        const std::string &dimension);

protected:
    int64_t GetParallelSizeByType(const std::string& type) const;
    void CalStrategyConfig(const std::string &tmpDimension, const ParallelStrategyConfig &tmpConfig);

    // get arrangements
    void ClearArrangementData();
    bool UpdateShowMap(std::string &err);
    void SetParaDetail(const std::string &para, uint32_t size);
    void UpdateElementSize();
    std::string GetElementName(std::unordered_map<std::string, uint32_t> &indexAttributes);
    Position GetElementPosition(std::unordered_map<std::string, uint32_t>& indexAttributes) const;

    // get performance indicator
    void SetTpIndicatorAttr();
    void SetPpIndicatorAttr();
    void SetCpIndicatorAttr();
    void SetDpIndicatorAttr();

    // get performance data
    virtual void CalculatePerformanceDataWithTpDimension(
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        std::vector<IndicatorDataStruct> &indicatorData);
    void ReduceTpPerformance(const std::unordered_map<std::uint32_t, StepStatistic> &statistic);
    virtual void CalculatePerformanceDataWithCpDimension(std::vector<IndicatorDataStruct> &indicatorData);
    void ReduceCpPerformance();
    void CalculatePerformanceDataWithPpDimension(std::vector<IndicatorDataStruct> &indicatorData);
    void ReducePpPerformanceForPpLast();
    void ReducePpPerformance(uint32_t startIndex, uint32_t step, uint32_t& dpGroupIdx);
    static void GetPerformanceResponseDataWithDpDimension(
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        std::vector<IndicatorDataStruct> &indicatorData);
    static double Reserved3DecimalPlaces(double num);
    static void AnalyzePerformanceAdviceWithDpCpPpTpDimension(
        Protocol::TraceStatistic &max, Protocol::TraceStatistic &min, double meanE2ETime,
        std::vector<std::string> &advices);

    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> ReduceCommDefaultFunc(
        const std::unordered_map<std::string, std::vector<CommInfoUnderRank>> &input, uint32_t w, uint32_t h);

    ParallelStrategyConfig strategyConfig;
    std::string dimension = DIMENSIONS_DP; // 默认一层级
    uint32_t wordSize = 1;
    uint32_t tpSize = 1;
    uint32_t tpCpSize = 1;
    uint32_t tpCpDpSize = 1;
    uint32_t tpCpPpSize = 1;

    // 折叠视图
    uint32_t elementSize = 1;
    uint32_t foldedTpSize = 1;
    uint32_t foldedTpCpSize = 1;
    uint32_t foldedTpCpDpSize = 1;
    uint32_t foldedTpCpPpSize = 1;

    ArrangementAndConnectionData data;

    std::vector<std::string> paraOrder{};
    std::vector<std::string> paraOrderWithEp{};
    std::vector<std::string> updatedOrder{};
    std::vector<std::string> updatedOrderWithEp{};
    std::vector<uint32_t> parallelSize{};
    std::vector<uint32_t> parallelSizeWithEp{}; // 仅影响连线生成，不影响布局
    std::vector<Connection> allCommunicationGroups{}; // 全量通信域
    std::unordered_map<std::string, ParallelDetails> paraDetailsMap; // 记录某并行域是否被折叠

    // get performance data
    std::unordered_map<std::uint32_t, StepStatistic> reduceTpMax;
    std::unordered_map<std::uint32_t, StepStatistic> reduceTpMin;
    std::unordered_map<std::uint32_t, StepStatistic> reduceCpMax;
    std::unordered_map<std::uint32_t, StepStatistic> reduceCpMin;
    std::unordered_map<std::uint32_t, StepStatistic> reducePpStatistic;
    const static inline int cpSizeWithEp = 1;
    const static inline int reservedNum = 2; // 保留2位小数
    const static inline int epPosPpLast = 2; // tp-cp-ep-dp-pp

    using CommInfoHandler = std::function<std::unordered_map<std::string, std::vector<CommInfoUnderRank>>(
        const std::unordered_map<std::string, std::vector<CommInfoUnderRank>>&)>;
    std::map<std::string, CommInfoHandler> commInfoHandlers;
    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> ReduceCommTpDimensionDef(
        const std::unordered_map<std::string, std::vector<CommInfoUnderRank>>& expendData);
    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> ReduceCommCpDimensionDef(
        const std::unordered_map<std::string, std::vector<CommInfoUnderRank>>& expendData);
    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> ReduceCommPpDimensionDef(
        const std::unordered_map<std::string, std::vector<CommInfoUnderRank>>& expendData);
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H
