/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_SOURCE_DEFS_H
#define PROFILER_SERVER_SOURCE_DEFS_H

namespace Dic {
namespace Module {
namespace Source {
struct ColumDataType {
    enum Type : int {
        SKIP = 0,
        INT,
        FLOAT,
        STRING
    };
};
}
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_SOURCE_DEFS_H
