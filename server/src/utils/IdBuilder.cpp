/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */


#include "IdBuilder.h"

namespace Dic {
IdBuilder &IdBuilder::RequestIdBuilder()
{
    static IdBuilder builder;
    return builder;
}

IdBuilder &IdBuilder::ResponseIdBuilder()
{
    static IdBuilder builder;
    return builder;
}

IdBuilder &IdBuilder::EventIdBuilder()
{
    static IdBuilder builder;
    return builder;
}

IdBuilder &IdBuilder::SessionIdBuilder()
{
    static IdBuilder builder;
    return builder;
}

int IdBuilder::Build()
{
    std::unique_lock<std::mutex> lock(idMutex);
    if (id >= maxId) {
        id = 0;
    }
    return id++;
}
} // end of namespace Dic