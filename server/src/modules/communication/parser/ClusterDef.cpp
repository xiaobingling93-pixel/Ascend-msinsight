/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <utility>
#include "ProtocolMessage.h"
#include "ClusterDef.h"

namespace Dic::Module {
using namespace Dic::Protocol;
bool ParallelStrategyConfig::CheckParams(std::string &errorMsg) const
{
    // algorithm只允许为以下白名单之一
    if (std::find(ALGORITHMS_ALLOWED.begin(), ALGORITHMS_ALLOWED.end(), algorithm) == ALGORITHMS_ALLOWED.end()) {
        errorMsg = "[Summary] Algorithm is not allowed.";
        return false;
    }
    // ppSize, tpSize, dpSize, cpSize, epSize, moeSize的范围[1, 10000]
    if (!CheckBaseParams(errorMsg)) {
        return false;
    }
    if (algorithm == MINDSPEED_TP_CP_EP_DP_PP_ALG && dpSize * cpSize % epSize != 0) {
        // 对于MindSpeed, 检查dpSize * cpSize是否能被epSize整除
        errorMsg = "[Summary] The product of DP size and CP size must be evenly divided by EP Size.";
        return false;
    }
    if ((algorithm == MEGATRON_LM_TP_CP_EP_DP_PP_ALG || algorithm == MEGATRON_LM_TP_CP_PP_EP_DP_ALG)
        && dpSize % epSize != 0) {
        // 对于Megatron, 检查dpSize是否能被epSize整除
        errorMsg = "[Summary] DP size must be evenly divided by EP Size.";
        return false;
    }
    if (algorithm == MINDIE_LLM_TP_DP_EP_PP_MOETP_ALG && (moeTpSize * epSize != tpSize * dpSize)) {
        // 对于MindIE-LLM算法，需检查world_size = moeTpSize * epSize * ppSize = tpSize * dpSize * ppSize
        errorMsg = "[Summary] The product of MOE_TP size and EP size should match "
                   "the product of TP size and DP size for the MindIE-LLM.";
        return false;
    }
    // 对于vLLM算法，需检查dpSize * tpSize是否能被epSize整除, epSize是否能被tpSize整除
    if (algorithm == VLLM_TP_PP_DP_EP_ALG) {
        if (dpSize * tpSize % epSize != 0) {
            errorMsg = "[Summary] The product of DP size and TP size must be evenly divided by EP Size.";
            return false;
        } else if (epSize % tpSize != 0) {
            errorMsg = "[Summary] EP size must be evenly divided by TP Size.";
            return false;
        }
    }
    uint64_t tmpProduct = static_cast<uint64_t>(dpSize) * cpSize * tpSize * ppSize;
    if (tmpProduct > MAX_WORLD_SIZE) {
        errorMsg = "[Summary] The product of PP size, TP size, DP size, and CP size must be less than " +
            std::to_string(MAX_WORLD_SIZE);
        return false;
    }
    return CheckParamForMindSpeed(errorMsg);
}

bool ParallelStrategyConfig::CheckBaseParams(std::string& errorMsg) const
{
    // 检查ppSize, tpSize, dpSize的范围
    if (ppSize <= 0 || ppSize > MAX_PARALLEL_SIZE) {
        errorMsg = "[Summary] PP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
        return false;
    }
    if (tpSize <= 0 || tpSize > MAX_PARALLEL_SIZE) {
        errorMsg = "[Summary] TP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
        return false;
    }
    if (dpSize <= 0 || dpSize > MAX_PARALLEL_SIZE) {
        errorMsg = "[Summary] DP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
        return false;
    }
    if (cpSize <= 0 || cpSize > MAX_PARALLEL_SIZE) {
        errorMsg = "[Summary] CP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
        return false;
    }
    if (epSize <= 0 || epSize > MAX_PARALLEL_SIZE) {
        errorMsg = "[Summary] EP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
        return false;
    }
    if (moeTpSize <= 0 || moeTpSize > MAX_PARALLEL_SIZE) {
        errorMsg = "[Summary] MOE_TP size must be between 1 and " + std::to_string(MAX_PARALLEL_SIZE);
        return false;
    }
    if (algorithm == MINDIE_LLM_TP_DP_EP_PP_MOETP_ALG && cpSize != 1) {
        errorMsg = "[Summary] The CP Parallelism is not supported by the MOE algorithm.";
        return false;
    }
    return true;
}

// LCOV_EXCL_BR_START
bool ParallelStrategyConfig::CheckParamForMindSpeed(std::string& errorMsg) const
{
    if (configForMindSpeed.cpAlgo.empty()) {
        return true;
    }
    // 如果cpAlgo不为空，只允许为以下四者之一
    if (std::find(MINDSPEED_CP_ALGORITHM_ALLOWED.begin(), MINDSPEED_CP_ALGORITHM_ALLOWED.end(),
                  configForMindSpeed.cpAlgo) == MINDSPEED_CP_ALGORITHM_ALLOWED.end()) {
        errorMsg = "[Summary] Mindspeed CP algorithm is not allowed.";
        return false;
    }
    // 检查tpSize是否能被nd1dim1 nd2dim1整除
    if (!CheckTp2DSizeForMindSpeed(errorMsg)) {
        return false;
    }
    // 检查cpSize是否能被ulyssesDegree整除
    if (configForMindSpeed.cpAlgo == MINDSPEED_HYBIRD_CP_ALG ||
        configForMindSpeed.cpAlgo == MINDSPEED_HYBIRD_ADAPTIVE_CP_ALG) {
        if (configForMindSpeed.ulyssesDegree <= 0) {
            errorMsg = "[Summary] Ulysses degree must be greater than 0.";
            return false;
        }
        if (cpSize % configForMindSpeed.ulyssesDegree != 0) {
            errorMsg = "[Summary] CP size must be evenly divided by ulysses degree for hybird cp.";
            return false;
        }
    }
    // 检查winSize
    if (!configForMindSpeed.useTp2D && configForMindSpeed.cpAlgo == MINDSPEED_HYBIRD_CP_ALG) {
        if (configForMindSpeed.winSize <= 0) {
            errorMsg = "[Summary] CP Window size must be greater than 0.";
            return false;
        }
        if (cpSize % (configForMindSpeed.ulyssesDegree * configForMindSpeed.winSize) != 0) {
            errorMsg = "[Summary] CP size must be evenly divided by the product of ulysses degree and cp window size.";
            return false;
        }
    }
    if (!configForMindSpeed.useTp2D && configForMindSpeed.cpAlgo == MINDSPEED_MEGATRON_CP_ALG) {
        if (configForMindSpeed.winSize <= 0) {
            errorMsg = "[Summary] CP Window size must be greater than 0.";
            return false;
        }
        if (cpSize % configForMindSpeed.winSize != 0) {
            errorMsg = "[Summary] CP size must be evenly divided by cp window size.";
            return false;
        }
    }
    return true;
}

bool ParallelStrategyConfig::CheckTp2DSizeForMindSpeed(std::string& errorMsg) const
{
    if (configForMindSpeed.useTp2D) {
        if (configForMindSpeed.nd1dim1 <= 0 || configForMindSpeed.nd2dim1 <= 0) {
            errorMsg = "[Summary] Nd1dim1 or nd2dim1 must be greater than 0.";
            return false;
        }
        if (tpSize % configForMindSpeed.nd1dim1 != 0 ||
            tpSize % configForMindSpeed.nd2dim1 != 0) {
            errorMsg = "[Summary] TP size must be evenly divided by nd1dim1 and nd2dim1 for tp2d.";
            return false;
        }
    }
    return true;
}
// LCOV_EXCL_BR_STOP
}
