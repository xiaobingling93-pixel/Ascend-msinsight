/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#ifndef PROFILER_SERVER_RLPROTOCOLRESPONSE_H
#define PROFILER_SERVER_RLPROTOCOLRESPONSE_H

#include <vector>
#include "ProtocolUtil.h"
#include "ProtocolDefs.h"

namespace Dic::Protocol {

struct RLPipelineNode {
    std::string fileId;
    std::string nodeType;
    uint64_t startTime = 0;
    uint64_t duration = 0;
    std::string name;
    std::string stageType;
};

struct RLPipelineItem {
    std::vector<RLPipelineNode> lists;
    std::string rankId;
    std::string hostName;
};

struct RLPipelineBody {
    uint64_t minTime = UINT64_MAX;
    uint64_t maxTime = 0;
    std::vector<RLPipelineItem> taskData;
    std::vector<RLPipelineItem> microBatchData;
    std::vector<std::string> stageTypeList;
    std::string backendType;
    std::string framework;
};

struct RLPipelineResponse : public Response {
    RLPipelineResponse() : Response(REQ_REQ_RL_PIPELINE) {}
    RLPipelineBody body;
};
}

#endif // PROFILER_SERVER_RLPROTOCOLRESPONSE_H
