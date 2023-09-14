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
    double timesTamp = 0;
    double totalAllocated = 0;
    double totalReserved = 0;
    std::string deviceType;
};

struct Operator {
    std::string name;
    double size = 0;
    double allocationTime = 0;
    double releaseTime = 0;
    double duration = 0;
    std::string deviceType;
};

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYDEF_H
