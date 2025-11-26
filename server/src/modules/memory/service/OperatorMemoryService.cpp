/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ServerLog.h"
#include "OperatorMemoryService.h"
namespace Dic::Module::Memory {
OperatorMemoryService::OperatorMemoryService(std::shared_ptr<OperatorTable> operatorTablePtr,
    std::shared_ptr<OpMemoryTable> opMemoryTablePtr)
    : operatorTable(std::move(operatorTablePtr)), opMemoryTable(std::move(opMemoryTablePtr))
{}

OperatorDomain OperatorMemoryService::ComputeAllocationTimeById(const std::string &rankId, const std::string &id)
{
    OperatorDomain target;
    std::vector<OperatorPO> textPOs;
    operatorTable->Select(OpMemoryColumn::ALLOCATION_TIME)
        .Eq(OpMemoryColumn::ID, id)
        .OrderBy(OpMemoryColumn::ALLOCATION_TIME, Timeline::TableOrder::ASC)
        .ExcuteQuery(rankId, textPOs);
    if (!std::empty(textPOs)) {
        target.allocationTime = textPOs.front().allocationTime;
        target.metaType = "TEXT";
        return target;
    }
    std::vector<OpMemoryPO> dbPOs;
    opMemoryTable->Select(OpMemoryColumn::ALLOCATION_TIME)
        .Eq(OpMemoryColumn::ID, id)
        .OrderBy(OpMemoryColumn::ALLOCATION_TIME, Timeline::TableOrder::ASC)
        .ExcuteQuery(rankId, dbPOs);
    if (!std::empty(dbPOs)) {
        target.allocationTime = dbPOs.front().allocationTime;
        target.metaType = "PYTORCH_API";
        return target;
    }
    Dic::Server::ServerLog::Warn("Failed to query operator allocation time, id is: ", id);
    return target;
}
}
