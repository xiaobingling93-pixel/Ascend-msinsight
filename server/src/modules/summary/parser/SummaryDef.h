/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARY_DEF_H
#define PROFILER_SERVER_SUMMARY_DEF_H

#include <string>
#include <optional>

namespace Dic {
namespace Module {
namespace Summary {

struct Kernel {
    std::string stepId;
    std::string name;
    std::string type;
    std::string acceleratorCore;
    double startTime;
    double duration;
    double waitTime;
    double blockDim;
    std::string inputShapes;
    std::string inputDataTypes;
    std::string inputFormats;
    std::string outputShapes;
    std::string outputDataTypes;
    std::string outputFormats;
};

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

#endif //PROFILER_SERVER_SUMMARY_DEF_H
