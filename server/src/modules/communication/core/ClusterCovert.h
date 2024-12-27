/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTERCOVERT_H
#define PROFILER_SERVER_CLUSTERCOVERT_H

#include "CommunicationProtocolResponse.h"
#include "ClusterDomainObject.h"

namespace Dic {
namespace Module {
namespace Communication {
class ClusterCovert {
public:
    static inline Protocol::MatrixData CovertMatrixDoToInfo(const MatrixInfoDo &matrixDo)
    {
        Protocol::MatrixData matrix;
        matrix.transitSize = matrixDo.transitSize;
        matrix.transitTime = matrixDo.transitTime;
        matrix.transportType = matrixDo.transportType;
        matrix.opName = matrixDo.opName;
        matrix.bandwidth = matrixDo.bandwidth;
        return matrix;
    }

    static inline Protocol::OperatorTimeItem CovertDoToOperatorTime(const OperatorTimeDo &operatorTimeDo)
    {
        Protocol::OperatorTimeItem operatorTimeItem;
        operatorTimeItem.operatorName = operatorTimeDo.operatorName;
        operatorTimeItem.startTime = operatorTimeDo.startTime;
        operatorTimeItem.elapseTime = operatorTimeDo.elapseTime;
        return operatorTimeItem;
    }

    static inline Protocol::DurationData CovertDoToDuration(const DurationDo &durationDo)
    {
        Protocol::DurationData duration;
        duration.startTime = durationDo.startTime;
        duration.elapseTime = durationDo.elapseTime;
        duration.transitTime = durationDo.transitTime;
        duration.synchronizationTime = durationDo.synchronizationTime;
        duration.waitTime = durationDo.waitTime;
        duration.idleTime = durationDo.idleTime;
        duration.synchronizationTimeRatio = durationDo.synchronizationTimeRatio;
        duration.waitTimeRatio = durationDo.waitTimeRatio;
        duration.sdmaBw = durationDo.sdmaBw;
        duration.rdmaBw = durationDo.rdmaBw;
        duration.sdmaTime = durationDo.sdmaTime;
        duration.rdmaTime = durationDo.rdmaTime;
        return duration;
    }
};
}
}
}
#endif // PROFILER_SERVER_CLUSTERCOVERT_H
