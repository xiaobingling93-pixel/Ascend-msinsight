/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#ifndef PROFILER_SERVER_SUMMARY_DEF_H
#define PROFILER_SERVER_SUMMARY_DEF_H

#include <string>

namespace Dic {
namespace Module {
namespace Summary {

// pmu数据表头仅允许字母  数字  '-'  '_'
// 为防止出现连续--(sql中为注释)导致sql注入, 增加负向前瞻
const std::string PMU_HEADER_WHITE_LIST_REG = R"(^(?!.*--)[a-zA-Z0-9\s\-_]+$)";

struct Kernel {
    std::string rankId;
    std::string stepId;
    std::string taskId;
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
