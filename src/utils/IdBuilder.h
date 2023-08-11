/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_ID_BUILDER_H
#define DATA_INSIGHT_CORE_ID_BUILDER_H

#include <mutex>

namespace Dic {
class IdBuilder {
public:
    static IdBuilder &RequestIdBuilder();
    static IdBuilder &ResponseIdBuilder();
    static IdBuilder &EventIdBuilder();
    static IdBuilder &SessionIdBuilder();
    int Build();

private:
    IdBuilder() = default;
    ~IdBuilder() = default;

    const int MAX_ID = 0x7dffffff;
    int id;
    std::mutex idMutex;
};
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_ID_BUILDER_H