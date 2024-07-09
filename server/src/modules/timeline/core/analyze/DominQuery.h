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
    /**
     * 界面选取的开始时间
     */
    uint64_t startTime = 0;
    /**
     * 界面选取的结束时间
     */
    uint64_t endTime = 0;
    /**
     * 泳道在数据库中的最小时间
     */
    uint64_t minTimestamp = 0;
    std::string cat;
    std::string cardId;
    bool isFilterPythonFunction = false;
    void QueryThreadsCheck(std::string &error) const
    {
        if (db == nullptr) {
            error = "database connection is not open";
            return;
        }
        if (startTime > endTime) {
            error = "start time is bigger than end time";
            return;
        }
        if (trackId == 0) {
            error = "track id is not correct";
        }
    }
};
}
#endif // PROFILER_SERVER_SLICEQUERY_H
