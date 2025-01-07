/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H

#include <unordered_map>
#include "SummaryProtocolResponse.h"
#include "BaseParallelStrategyAlgorithm.h"

namespace Dic::Module {

struct ParallelDetails {
    bool isShown = false;  // shownMap 代表当前层次是否展示
    uint32_t size = 1; // sizeMap ppSize等。若当前层次已折叠，size置1
};

class MegatronParallelStrategyAlgorithm : public BaseParallelStrategyAlgorithm {
public:
    MegatronParallelStrategyAlgorithm();
    ~MegatronParallelStrategyAlgorithm() override;

    void ClearStrategyConfigCache() override;
    bool UpdateParallelDimension(const std::string &tmpDimension,
                                 const ParallelStrategyConfig &tmpConfig, std::string &err) override;

    void GenerateArrangementByDimension() override;
    ArrangementAndConnectionData GetArrangementData() override;
    bool GetPerformanceIndicatorByDimension(const Protocol::ParallelismPerformance &performanceParams,
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        PerformanceIndicatorData &performanceResponseData, std::string& err) override;

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
    void GetConnections(Element &curEle);
    void AddConnection(std::vector<Connection> &connections, const std::string &paraType, uint32_t len,
                       uint32_t stepSize, Element &curEle);
    // get performance data
    void SortPerformanceDataByIndex(std::vector<IndicatorDataStruct>& performanceData);
    void CalculatePerformanceDataWithDpCpPpTpDimension(
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        PerformanceIndicatorData &performanceResponseData);
    void CalculatePerformanceDataWithPpDimension(PerformanceIndicatorData &indicatorData);
    void ReduceTpPerformance(const std::unordered_map<std::uint32_t, StepStatistic> &statistic);
    void GetPerformanceResponseDataWithCollapsedDimension(
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        PerformanceIndicatorData &performanceResponseData);
    void ReducePpPerformanceForPpLast();
    void ReducePpPerformanceForDpLast();
    void ReducePpPerformance(uint32_t startIndex, uint32_t step, uint32_t& ppGroupIdx);
    void ReduceCpPerformance();
    double Reserved3DecimalPlaces(double num);

    ArrangementAndConnectionData data;
    uint32_t elementSize = 1;
    std::string dimension = DIMENSIONS_DP; // 默认一层级
    std::vector<std::string> paraOrder;
    std::unordered_map<std::string, ParallelDetails> paraDetailsMap; // 记录某并行域是否被折叠

    // get performance data
    uint32_t wordSize = 1;
    uint32_t tpSize = 1;
    uint32_t tpCpSize = 1;
    uint32_t tpCpDpSize = 1;
    uint32_t tpCpPpSize = 1;
    std::unordered_map<std::uint32_t, StepStatistic> reduceTpMax;
    std::unordered_map<std::uint32_t, StepStatistic> reduceTpMin;
    std::unordered_map<std::uint32_t, StepStatistic> reducePpStatistic;
    std::unordered_map<std::uint32_t, StepStatistic> reduceCpStatistic;
    const int numTwo = 2; // 保留2位小数
    void AnalyzePerformanceAdviceWithDpCpPpTpDimension(Protocol::TraceStatistic &max, Protocol::TraceStatistic &min,
                                                       double meanE2ETime, std::vector<std::string> &advices);
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
