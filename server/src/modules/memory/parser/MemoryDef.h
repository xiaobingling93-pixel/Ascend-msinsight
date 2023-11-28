/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORYDEF_H
#define PROFILER_SERVER_MEMORYDEF_H

#include <string>

namespace Dic {
namespace Module {
namespace Memory {

struct Record {
    std::string component;
    double timesTamp;
    double totalAllocated;
    double totalReserved;
    std::string deviceType;
};

struct Operator {
    std::string name;
    double size;
    double allocationTime;
    double releaseTime;
    double duration;
    std::string deviceType;
};

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYDEF_H
