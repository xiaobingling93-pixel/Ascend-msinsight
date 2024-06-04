/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_SLICEQUERY_H
#define PROFILER_SERVER_SLICEQUERY_H
#include <string>
#include "sqlite3.h"
namespace Dic::Module::Timeline {
struct SliceQuery {
    sqlite3 *db;
    uint64_t trackId = 0;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    uint64_t minTimestamp = 0;
    std::string cat;
    std::string cardId;
    bool isFilterPythonFunction = false;
};
}
#endif // PROFILER_SERVER_SLICEQUERY_H
