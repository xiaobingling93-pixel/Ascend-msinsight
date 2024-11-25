/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARY_DEF_H
#define PROFILER_SERVER_SUMMARY_DEF_H

#include <string>

namespace Dic {
namespace Module {
namespace Summary {

struct Kernel {
    std::string rankId;
    std::string stepId;
    std::string name;
    std::string type;
    std::string state;
    std::string acceleratorCore;
    int64_t startTime;
    double duration;
    double waitTime;
    int64_t blockDim;
    std::string inputShapes;
    std::string inputDataTypes;
    std::string inputFormats;
    std::string outputShapes;
    std::string outputDataTypes;
    std::string outputFormats;
    std::vector<std::string> utilizationInfo;
};

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_SUMMARY_DEF_H
