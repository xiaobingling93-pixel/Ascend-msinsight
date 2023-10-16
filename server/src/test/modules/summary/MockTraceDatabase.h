/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MOCKTRACEDATABASE_H
#define PROFILER_SERVER_MOCKTRACEDATABASE_H

#include <gmock/gmock.h>
#include "TraceDatabase.h"
#include "SummaryProtocolRequest.h"

class MockTraceDatabase : public Dic::Module::Timeline::TraceDatabase {
public:
    MOCK_METHOD2(QueryComputeStatisticsData, bool(const Dic::Protocol::SummaryStatisticParams &requestParams,
            Dic::Protocol::SummaryStatisticsBody &responseBody));
};

#endif //PROFILER_SERVER_MOCKTRACEDATABASE_H
