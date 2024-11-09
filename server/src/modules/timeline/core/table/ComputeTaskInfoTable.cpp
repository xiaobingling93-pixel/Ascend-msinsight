/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ComputeTaskInfoTable.h"
namespace Dic::Module::Timeline {
void ComputeTaskInfoTable::SetName(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.name = resultSet->GetUint64(ComputeTaskInfoColumn::NAME);
}

void ComputeTaskInfoTable::SetGlobalTaskId(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.globalTaskId = resultSet->GetUint64(ComputeTaskInfoColumn::GLOBAL_TASK_ID);
}

void ComputeTaskInfoTable::SetBlockDim(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.blockDim = resultSet->GetUint64(ComputeTaskInfoColumn::BLOCK_DIM);
}

void ComputeTaskInfoTable::SetMixBlockDim(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.mixBlockDim = resultSet->GetUint64(ComputeTaskInfoColumn::MIX_BLOCK_DIM);
}

void ComputeTaskInfoTable::SetTaskType(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.taskType = resultSet->GetUint64(ComputeTaskInfoColumn::TASK_TYPE);
}

void ComputeTaskInfoTable::SetOpType(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.opType = resultSet->GetUint64(ComputeTaskInfoColumn::OP_TYPE);
}

void ComputeTaskInfoTable::SetInputFormats(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.inputFormats = resultSet->GetUint64(ComputeTaskInfoColumn::INPUT_FORMATS);
}

void ComputeTaskInfoTable::SetInputDataTypes(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.inputDataTypes = resultSet->GetUint64(ComputeTaskInfoColumn::INPUT_DATA_TYPES);
}

void ComputeTaskInfoTable::SetInputShapes(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.inputShapes = resultSet->GetUint64(ComputeTaskInfoColumn::INPUT_SHAPES);
}

void ComputeTaskInfoTable::SetOutputFormats(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.outputFormats = resultSet->GetUint64(ComputeTaskInfoColumn::OUTOUT_FORMATS);
}

void ComputeTaskInfoTable::SetOutputDataTypes(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.outputDataTypes = resultSet->GetUint64(ComputeTaskInfoColumn::OUTPUT_DATA_TYPES);
}

void ComputeTaskInfoTable::SetOutputShapes(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.outputShapes = resultSet->GetUint64(ComputeTaskInfoColumn::OUTPUT_SHAPES);
}

void ComputeTaskInfoTable::SetAttrInfo(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.attrInfo = resultSet->GetUint64(ComputeTaskInfoColumn::ATTRINFO);
}

void ComputeTaskInfoTable::SetWaitNs(ComputeTaskInfoPO &computeTaskInfoPO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    computeTaskInfoPO.waitNs = resultSet->GetUint64(ComputeTaskInfoColumn::WAIT_NS);
}
}
