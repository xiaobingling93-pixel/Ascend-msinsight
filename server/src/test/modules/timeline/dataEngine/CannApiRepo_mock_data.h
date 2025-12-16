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