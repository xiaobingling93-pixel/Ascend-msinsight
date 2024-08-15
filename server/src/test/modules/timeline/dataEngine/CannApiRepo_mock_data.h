/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CANNAPIREPO_MOCK_DATA_H
#define PROFILER_SERVER_CANNAPIREPO_MOCK_DATA_H
#include <DominQuery.h>
#include "CannApiTable.h"
using namespace Dic::Module::Timeline;
namespace Dic::TimeLine::CannApiRepo::Mock {
void QueryCompeteSliceByIdsWithNormalExcuteQuery(const std::string &fileId, std::vector<CannApiPO> &result)
{
    CannApiPO cannApiPO1 = { 1, 22, 45, 0, 0, 0, 0 };
    CannApiPO cannApiPO2 = { 2, 23, 50, 0, 0, 0, 0 };
    result.emplace_back(cannApiPO1);
    result.emplace_back(cannApiPO2);
}

void QuerySimpleSliceWithOutNameByTrackIdWithNormalExcuteQuery(const std::string &fileId,
    std::vector<CannApiPO> &result)
{
    CannApiPO cannApiPO1 = { 1, 22, 45, 0, 0, 2, 0 };
    CannApiPO cannApiPO2 = { 2, 23, 50, 0, 0, 3, 0 };
    result.emplace_back(cannApiPO1);
    result.emplace_back(cannApiPO2);
}
}
#endif // PROFILER_SERVER_CANNAPIREPO_MOCK_DATA_H