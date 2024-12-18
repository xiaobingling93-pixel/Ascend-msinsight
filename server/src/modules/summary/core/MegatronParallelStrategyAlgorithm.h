/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H

#include <unordered_map>
#include "BaseParallelStrategyAlgorithm.h"

namespace Dic::Module {

const std::vector<std::string> LAYOUT = {DP_PARA, CP_PARA, PP_PARA, TP_PARA};
struct ParallelDetails {
    bool isShown = false;  // shownMap 代表当前层次是否展示
    uint32_t size = 1; // sizeMap ppSize等。若当前层次已折叠，size置1
};

class MegatronParallelStrategyAlgorithm : public BaseParallelStrategyAlgorithm {
public:
    MegatronParallelStrategyAlgorithm();
    ~MegatronParallelStrategyAlgorithm() override;

    bool UpdateParallelDimension(const std::string &tmpDimension,
                                 const ParallelStrategyConfig &tmpConfig, std::string &err) override;

    void GenerateArrangementByDimension() override;
    ArrangementAndConnectionData GetArrangementData() override;

private:
    // get arrangements
    void ClearArrangementData();
    void UpdateElementSize();
    bool UpdateShowMap(std::string &err);
    void SetParaDetail(const std::string &para, int64_t size);
    void SetIndicatorAttr();
    void GetPerArrangement(uint32_t index, std::unordered_map<std::string, uint32_t> &indexAttributes);
    void UpdateIndexAttributes(std::unordered_map<std::string, uint32_t> &indexAttributes);
    std::string GetElementName(std::unordered_map<std::string, uint32_t> &indexAttributes);
    Position GetElementPosition(std::unordered_map<std::string, uint32_t>& indexAttributes);
    // get connections
    void GetConnections(Element &curEle);
    void AddConnection(std::vector<Connection> &connections, const std::string &paraType, uint32_t len,
                       uint32_t stepSize, Element &curEle);

    ArrangementAndConnectionData data;
    uint32_t elementSize = 1;
    ParallelStrategyConfig config;
    std::string dimension = DIMENSIONS_DP; // 默认一层级
    std::vector<std::string> paraOrder;
    std::unordered_map<std::string, ParallelDetails> paraDetailsMap; // 记录某并行域是否被折叠
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
