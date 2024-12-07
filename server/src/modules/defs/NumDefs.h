/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: defines declaration
 */

#ifndef PROFILER_SERVER_NUMDEFS_H
#define PROFILER_SERVER_NUMDEFS_H

namespace Dic {

    // 数据库执行插入语句前最大缓存数量
    const static uint32_t TABLE_CACHE_SIZE = 1000;

    const static uint32_t MB_SIZE = 1024 * 1024;

    const static uint32_t KB_SIZE = 1024;

    const static int INT_TWO = 2;

    const static int INT_TEN = 10;

    // 是否立即解析Trace view的临界值
    const static int PENDIND_CRITICAL_VALUE = 17;

    const static int64_t MIN_PAGESIZE = 0;
    const static int64_t MAX_PAGESIZE = 1000;
    const static int64_t DEFAULT_PAGESIZE = 10;
    const static int64_t MIN_CURRENT_PAGE = 0;
    const static int64_t MAX_CURRENT_PAGE = 10000000000;
} // end of namespace Dic


#endif // PROFILER_SERVER_NUMDEFS_H
