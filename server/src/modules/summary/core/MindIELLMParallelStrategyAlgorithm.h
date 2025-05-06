/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_MINDIELLMPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_MINDIELLMPARALLELSTRATEGYALGORITHM_H

#include "BaseParallelStrategyAlgorithm.h"
#include "ParallelStrategyAlgorithmHelper.h"
#include "ParallelStrategyAlgorithmDef.h"

namespace Dic::Module::Summary {
using CommInfoMap = std::unordered_map<std::string, std::vector<CommInfoUnderRank>>;
class MindIELLMParallelStrategyAlgorithm : public BaseParallelStrategyAlgorithm {
public:
    static const std::unordered_map<std::string, std::string> tokenExceptEp;
    static const std::unordered_map<std::string, std::string> tokenWithEp;
    MindIELLMParallelStrategyAlgorithm();
    ~MindIELLMParallelStrategyAlgorithm() override;
    bool UpdateParallelDimension(const std::string& tmpDimension,
                                 const ParallelStrategyConfig& tmpConfig, std::string& err) override;
    bool GenerateArrangementByDimension(std::string &err) override;
    bool GetPerformanceIndicatorByDimension(const GetPerformanceIndicatorParam &performanceParams,
                                            const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
                                            std::vector<IndicatorDataStruct> &indicatorData, std::string& err) override;
    void CalAdviceInfo(const std::string &dimension, std::vector<std::string> &advices,
                       std::vector<IndicatorDataStruct> &indicatorData) override;
    std::vector<Connection> GetAllCommunicationGroups(std::string &err) override;
    CommInfoMap GetCommInfoByDimension(const CommInfoMap &expandCommInfos, const std::string &dimension) override;
private:
    enum class ParaMode {
        TP_DP_PP,
        MOE_TP_EP_PP
    };
    void UpdateOrderAndParallelSize();
    void SetIndicatorAttr();
    void GetPerArrangement(uint32_t index, std::unordered_map<std::string, uint32_t> &indexAttributes);
    void UpdateIndexAttributes(std::unordered_map<std::string, uint32_t> &indexAttributes);
    bool GetConnectionsByTokenList(std::string &err);
    bool GetConnectionsByToken(std::string &err, ParaMode mode);
};
}

#endif // PROFILER_SERVER_MINDIELLMPARALLELSTRATEGYALGORITHM_H
