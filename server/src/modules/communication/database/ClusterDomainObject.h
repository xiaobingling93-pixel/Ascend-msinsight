/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTERDOMAINOBJECT_H
#define PROFILER_SERVER_CLUSTERDOMAINOBJECT_H

#include <string>
#include <vector>
#include "CommunicationProtocolResponse.h"

namespace Dic {
namespace Module {
    struct MatrixInfoDo {
        int srcRank;
        int dstRank;
        std::string transportType;
        std::string opName;
        double transitSize;
        double transitTime;
        double bandwidth;
    };

    struct OperatorTimeDo {
        std::string rankId;
        std::string operatorName;
        uint64_t startTime = 0;
        uint64_t elapseTime = 0;
    };

    struct DurationDo {
        std::string rankId;
        double startTime;
        double elapseTime;
        double transitTime;
        double synchronizationTime;
        double waitTime;
        double idleTime;
        double synchronizationTimeRatio;
        double waitTimeRatio;
        double sdmaBw{};
        double rdmaBw{};
        double sdmaTime{};
        double rdmaTime{};
    };
}
}
#endif // PROFILER_SERVER_CLUSTERDOMAINOBJECT_H
