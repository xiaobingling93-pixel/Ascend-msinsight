/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_VLLMPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_VLLMPARALLELSTRATEGYALGORITHM_H

#include "BaseParallelStrategyAlgorithm.h"
#include "ParallelStrategyAlgorithmHelper.h"
#include "ParallelStrategyAlgorithmDef.h"

namespace Dic::Module::Summary {
class VLLMParallelStrategyAlgorithm : public BaseParallelStrategyAlgorithm {
public:
    static const std::unordered_map<std::string, std::string> tokenExceptEp;
    static const std::unordered_map<std::string, std::string> tokenWithEp;

    VLLMParallelStrategyAlgorithm();
    ~VLLMParallelStrategyAlgorithm() override;
    bool UpdateParallelDimension(const std::string& tmpDimension,
                                 const ParallelStrategyConfig& tmpConfig, std::string& err) override;
    void SetStrategyConfig(const ParallelStrategyConfig& config) override;
    bool GenerateArrangementByDimension(std::string &err) override;
    bool GetPerformanceIndicatorByDimension(const GetPerformanceIndicatorParam &performanceParams,
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        std::vector<IndicatorDataStruct> &indicatorData, std::string& err) override;
    std::vector<Connection> GetAllCommunicationGroups(std::string &err) override;
private:
    void UpdateOrderAndParallelSize();
    void SetIndicatorAttr();
    void GetPerArrangement(uint32_t index, std::unordered_map<std::string, uint32_t> &indexAttributes);
    void UpdateIndexAttributes(std::unordered_map<std::string, uint32_t> &indexAttributes);
    bool GetConnectionsByTokenList(std::string &err);
    bool GetConnectionsByToken(std::string &err, bool independentEp);
    uint32_t innerDpSize = 1;
    uint32_t externalDpSize = 1;
};
}

#endif // PROFILER_SERVER_VLLMPARALLELSTRATEGYALGORITHM_H
