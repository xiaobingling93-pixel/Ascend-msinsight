/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H

#include <unordered_map>
#include "BaseParallelStrategyAlgorithm.h"
#include "ParallelStrategyAlgorithmHelper.h"
#include "ParallelStrategyAlgorithmDef.h"

namespace Dic::Module::Summary {
class MegatronParallelStrategyAlgorithm : public BaseParallelStrategyAlgorithm {
public:

    static const inline std::unordered_map<std::string, std::string> tokenExceptEp = {
        {DP_GROUP, DP_GROUP}, {CP_GROUP, CP_GROUP}, {TP_GROUP, TP_GROUP}, {PP_GROUP, PP_GROUP},
        {DP_CP_GROUP, DP_CP_GROUP}, {MP_GROUP, MP_GROUP_NAME}, {TP_DP_CP_GROUP, TP_DP_CP_GROUP},
        {TP_DP_GROUP, TP_DP_GROUP}, {TP_CP_GROUP, TP_CP_GROUP}
    };
    static const inline std::unordered_map<std::string, std::string> tokenWithEp = {
        {EP_GROUP, EP_GROUP_NAME}, {TP_EP_GROUP, TP_EP_GROUP_NAME}, {DP_MODULO_EP_GROUP, DP_MODULO_EP_GROUP_NAME},
        {DP_CP_MODULO_EP_GROUP, DP_CP_MODULO_EP_GROUP_NAME}, {MP_EP_GROUP, MP_EP_GROUP_NAME}
    };

    MegatronParallelStrategyAlgorithm();
    ~MegatronParallelStrategyAlgorithm() override;

    bool UpdateParallelDimension(const std::string &tmpDimension,
                                 const ParallelStrategyConfig &tmpConfig, std::string &err) override;

    bool GenerateArrangementByDimension(std::string &err) override;
    bool GetPerformanceIndicatorByDimension(const GetPerformanceIndicatorParam &performanceParams,
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic,
        std::vector<IndicatorDataStruct> &indicatorData, std::string& err) override;
    void CalAdviceInfo(const std::string &dimension, std::vector<std::string> &advices,
                       std::vector<IndicatorDataStruct> &indicatorData) override;
    // get all communication groups
    std::vector<Connection> GetAllCommunicationGroups(std::string &err) override;
private:
    // get arrangements
    void SetIndicatorAttr();
    void GetPerArrangement(uint32_t index, std::unordered_map<std::string, uint32_t> &indexAttributes);
    void UpdateIndexAttributes(std::unordered_map<std::string, uint32_t> &indexAttributes);
    std::vector<uint32_t> GetElementContainRanks(uint32_t index,
                                                 std::unordered_map<std::string, uint32_t> &indexAttributes);
    // get connections
    void UpdateOrderAndParallelSize();
    bool GetConnectionsByMegatronToken(std::string &err, bool independentEp);
    bool GetConnectionsByTokenList(std::string &err);
    void GetConnections(Element &curEle);
    void AddConnection(std::vector<Connection> &connections, const std::string &paraType, uint32_t len,
                       uint32_t stepSize, Element &curEle);
    // get performance data
    void ReducePpPerformanceForDpLast();
    const static inline int epPosDpLast = 3; // tp-cp-pp-ep-dp
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
