/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTER_DEF_H
#define PROFILER_SERVER_CLUSTER_DEF_H

#include <string>
#include <optional>
#include <fstream>
#include "ServerLog.h"

namespace Dic {
namespace Module {
struct CommunicationTimeInfo {
    std::string iterationId;
    std::string stageId;
    std::string rankId;
    std::string opName;
    std::string opSuffix;
    uint64_t startTime = 0;
    double elapseTime = 0;
    double synchronizationTimeRatio = 0;
    double synchronizationTime = 0;
    double transitTime = 0;
    double waitTimeRatio = 0;
    double waitTime = 0;
    double idleTime = 0;
};
struct CommunicationBandWidth {
    std::string iterationId;
    std::string rankId;
    std::string stageId;
    std::string opName;
    std::string opSuffix;
    std::string transportType;
    double bandwidthSize = 0;
    double bandwidthUtilization = 0;
    double largePackageRatio = 0;
    std::string sizeDistribution;
    double transitSize = 0;
    double transitTime = 0;
};
struct StepStatistic {
    std::string rankId;
    std::string stepId;
    std::string stageId;
    double computingTime = 0;
    double pureCommunicationTime = 0;
    double overlapCommunicationTime = 0;
    double communicationTime = 0;
    double freeTime = 0;
    double stageTime = 0;
    double bubbleTime = 0;
    double pureCommunicationExcludeReceiveTime = 0;
    double prepareTime = 0;
};

struct ClusterBaseInfo {
    std::string filePath;
    std::string ranks;
    std::string steps;
    std::string ppStages;
    std::string stages;
    long collectStartTime;
    double collectDuration;
    long long dataSize  =  0;
};

struct CommunicationMatrixInfo {
    std::string groupId;
    std::string iterationId;
    std::string sortOp;
    std::string opName;
    std::string groupName;
    int srcRank;
    int dstRank;
    std::string transportType;
    double transitSize;
    double transitTime;
    double bandwidth;
};

} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_CLUSTER_DEF_H