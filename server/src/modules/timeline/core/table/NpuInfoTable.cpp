/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "NpuInfoTable.h"

namespace Dic::Module::Timeline {
void NpuInfoTable::SetId(NpuInfoPo &npuInfoPo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    npuInfoPo.id = resultSet->GetUint64(NpuInfoColumn::ID);
}
void NpuInfoTable::SetName(NpuInfoPo &npuInfoPo, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    npuInfoPo.name = resultSet->GetString(NpuInfoColumn::NAME);
}
}