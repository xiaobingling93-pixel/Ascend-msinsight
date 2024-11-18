/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SAFEFILE_H
#define PROFILER_SERVER_SAFEFILE_H

#include "FileUtil.h"

namespace Dic {
    std::ifstream OpenReadFileSafely(const std::string &path, std::ios::openmode mode = std::ios::in);
}

#endif // PROFILER_SERVER_SAFEFILE_H
