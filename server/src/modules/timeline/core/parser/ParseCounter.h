/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSECOUNTER_H
#define PROFILER_SERVER_PARSECOUNTER_H

#include <atomic>

namespace Dic {
namespace Module {
namespace Timeline {
class ParseCounter {
public:
    static ParseCounter &Instance();
    int getCount();
    void addCount();
    void minusCount();
private:
    std::atomic<int> EXECUTING_RANK_COUNT = 0;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif //PROFILER_SERVER_PARSECOUNTER_H
