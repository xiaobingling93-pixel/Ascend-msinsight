/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARALLELSTRATEGYALGORITHMDEF_H
#define PROFILER_SERVER_PARALLELSTRATEGYALGORITHMDEF_H

#include <string>
namespace Dic::Module::Summary {

// Token list for Megatron and MindSpeed
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

// Token list for MindSpeed
// CP
const std::string CP_ULYSSES_GROUP = "ulysses";
const std::string CP_ULYSSES_GROUP_NAME = "cp_ulysses";
const std::string CP_RING_GROUP = "ring";
const std::string CP_RING_GROUP_NAME = "cp_ring";
const std::vector<std::string> ULYSSES_RING_TOKEN = {CP_ULYSSES_GROUP, CP_RING_GROUP};
const std::string CP_WIN_GROUP = "win";
const std::string CP_WIN_GROUP_NAME = "cp_ring_intra";
const std::string CP_INTER_GROUP = "inter";
const std::vector<std::string> WIN_INTRA_TOKEN = {CP_WIN_GROUP, CP_INTER_GROUP};

// 2D TP
const std::string TP_GROUP_FOR_ND1_DIM1 = "nd1dim1";
const std::string TP_GROUP_FOR_ND1_DIM1_NAME = "nd1_dim1";
const std::string TP_GROUP_FOR_ND1_DIM2 = "nd1dim2";
const std::string TP_GROUP_FOR_ND1_DIM2_NAME = "nd1_dim2";
const std::vector<std::string> TP2D_ND1_TOKEN = {TP_GROUP_FOR_ND1_DIM1, TP_GROUP_FOR_ND1_DIM2};
const std::string TP_GROUP_FOR_ND2_DIM1 = "nd2dim1";
const std::string TP_GROUP_FOR_ND2_DIM1_NAME = "nd2_dim1";
const std::string TP_GROUP_FOR_ND2_DIM2 = "nd2dim2";
const std::string TP_GROUP_FOR_ND2_DIM2_NAME = "nd2_dim2";
const std::vector<std::string> TP2D_ND2_TOKEN = {TP_GROUP_FOR_ND2_DIM1, TP_GROUP_FOR_ND2_DIM2};
}
#endif // PROFILER_SERVER_PARALLELSTRATEGYALGORITHMDEF_H
