 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  */

#ifndef PROFILER_SERVER_AICPUOPADVISOR_H
#define PROFILER_SERVER_AICPUOPADVISOR_H

#include <map>
#include <tuple>

#include "SummaryDef.h"
#include "VirtualTraceDatabase.h"
#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"

namespace Dic::Module::Advisor {
const uint64_t AICPU_OP_DURATION_THRESHOLD = Timeline::AICPU_OP_DURATION_THRESHOLD; // 20us

const std::vector<std::string> AICPU_OP_EQUIVALENT_REPLACE = {
    "index", "indexput", "indexputv2", "nonzero"
};
const std::map<std::string, Timeline::AICpuCheckDataType> AICPU_OP_DATATYPE_RULE = {
    {
        "cast",
        {
            std::vector<std::string>{"float", "float32", "float16", "bool", "int32",
                                     "uint32", "int64", "uint64", "uint8", "dt_bf16"},
            std::vector<std::string>{"float", "float32", "float16", "bool", "int32",
                                     "uint32", "int64", "uint64", "uint8", "dt_bf16"},
            ""
        }
    },
    {
        "tensorequal",
        {
            std::vector<std::string>{"float", "float32", "float16", "bool", "int32", "int8", "uint8"},
            std::vector<std::string>{"bool"},
            ""
        }
    },
    {
        "equal",
        {
            std::vector<std::string>{"float", "float32", "float16", "bool", "int32", "int64", "int8", "uint8"},
            std::vector<std::string>{"bool"},
            ""
        }
    },
    {
        "nonzero",
        {
            std::vector<std::string>{"float16", "bool", "dt_bf16"},
            std::vector<std::string>{"int64"},
            ""
        }
    },
    {
        "mul",
        {
            std::vector<std::string>{"float", "float32", "float16", "bool", "int32", "uint32",
                                     "int64", "uint64", "int8", "uint8", "dt_bf16"},
            std::vector<std::string>{"float", "float32", "float16", "bool", "int32", "uint32",
                                     "int64", "uint64", "int8", "uint8", "dt_bf16"},
            ""
        }
    },
    {
        "other",
        {
            std::vector<std::string>{"float", "float32", "float16", "bool", "int32",
                                     "uint32", "int64", "uint64", "int8", "uint8", "int16", "uint16", "dt_bf16"},
            std::vector<std::string>{"float", "float32", "float16", "bool", "int32",
                                     "uint32", "int64", "uint64", "int8", "uint8", "int16", "uint16", "dt_bf16"},
            ""
        }
    }
};

const std::vector<std::string> AICPU_OP_ORDER_BY_NAME_LIST = {
    "startTime", "duration", "pid", "tid", "name"
};

class AICpuOpAdvisor {
public:
    static bool Process(const Protocol::APITypeParams& params, Protocol::AICpuOperatorResBody& resBody);

private:
    static std::string GenerateAICpuOperatorNote(const Protocol::KernelBaseInfo& info);
};

} // Dic::Module::Advisor

#endif // PROFILER_SERVER_AICPUOPADVISOR_H
