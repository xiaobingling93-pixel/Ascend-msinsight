/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "ParseCounter.h"

namespace Dic {
namespace Module {
namespace Timeline {
ParseCounter &ParseCounter::Instance()
{
    static ParseCounter instance;
    return instance;
}

int ParseCounter::getCount()
{
    return EXECUTING_RANK_COUNT;
}

void ParseCounter::addCount()
{
    EXECUTING_RANK_COUNT++;
}

void ParseCounter::minusCount()
{
    EXECUTING_RANK_COUNT--;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic