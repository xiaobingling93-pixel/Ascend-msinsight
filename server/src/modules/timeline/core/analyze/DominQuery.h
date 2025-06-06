/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_SLICEQUERY_H
#define PROFILER_SERVER_SLICEQUERY_H
#include <string>
#include "DomainObject.h"
namespace Dic::Module::Timeline {
struct SliceQuery {
    uint64_t trackId = 0;
    /* *
     * 界面选取的开始时间
     */
    uint64_t startTime = 0;
    /* *
     * 界面选取的结束时间
     */
    uint64_t endTime = 0;
    /* *
     * 泳道在数据库中的最小时间
     */
    uint64_t minTimestamp = 0;
    PROCESS_TYPE metaType = PROCESS_TYPE::NONE;
    std::string cat;
    /*
     * timeline框选时使用
    */
    std::string rankId;
    std::string pid;
    std::string tid;
    std::string sliceId;
    /**
     * 算子名字
     */
    std::string name;
    /**
     *
     */
    uint64_t timePoint = 0;
    bool isFilterPythonFunction = false;
    bool QueryThreadsCheck(std::string &error) const
    {
        if (startTime > endTime) {
            error = "start time is bigger than end time";
            return false;
        }
        if (trackId == 0) {
            error = "track id is not correct";
            return false;
        }
        return true;
    }
};

struct SliceQueryByNameList {
    std::string rankId;
    std::string processName;
    std::vector<std::string> nameList;
    PROCESS_TYPE metaType = PROCESS_TYPE::NONE;
};

struct FlowQuery {
    uint64_t trackId = 0;
    /* *
     * 连线的id
     */
    std::string flowId;
    /* *
     * 界面选取的开始时间
     */
    uint64_t startTime = 0;
    /* *
     * 界面选取的结束时间
     */
    uint64_t endTime = 0;
    /* *
     * 泳道在数据库中的最小时间
     */
    uint64_t minTimestamp = 0;
    /**
     * 连线类别
     */
    std::string cat;
    std::string fileId;
    PROCESS_TYPE metaType = PROCESS_TYPE::NONE;
    /**
     * flowId集合
     */
    std::vector<std::string> flowIds;
};

struct ThreadQuery {
    std::string fileId;
    PROCESS_TYPE metaType = PROCESS_TYPE::NONE;
};

struct TrackQuery {
    /* *
     * 数据库db文件路径唯一标识
     */
    std::string rankId;
    /* *
     * 进程信息
     */
    std::string processId;
    /* *
     * 线程信息
     */
    std::string threadId;
    /* *
     * 线程泳道唯一标识
     */
    uint64_t trackId = 0;

    /**
     * 线程泳道属于哪个类型
     */
    std::string metaType;

    /**
     * 泳道开始时间
     */
    uint64_t startTime = 0;

    /**
     * 泳道结束时间
     */
    uint64_t endTime = 0;

    std::string fileId;
};
}
#endif // PROFILER_SERVER_SLICEQUERY_H
