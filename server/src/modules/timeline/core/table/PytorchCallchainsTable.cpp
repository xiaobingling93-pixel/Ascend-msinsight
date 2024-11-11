/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "PytorchCallchainsTable.h"
namespace Dic::Module::Timeline {
void PytorchCallchainsTable::SetId(PytorchCallchainsPO &pytorchApiPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    pytorchApiPO.id = resultSet->GetUint64(PytorchCallchainsColumn::ID);
}

void PytorchCallchainsTable::SetStack(PytorchCallchainsPO &pytorchApiPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    pytorchApiPO.stack = resultSet->GetUint64(PytorchCallchainsColumn::STACK);
}

void PytorchCallchainsTable::SetStackDepth(PytorchCallchainsPO &pytorchApiPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    pytorchApiPO.stackDepth = resultSet->GetUint64(PytorchCallchainsColumn::STACK_DEPTH);
}
}
