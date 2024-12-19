/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_OPERATORMEMORYSERVICE_H
#define PROFILER_SERVER_OPERATORMEMORYSERVICE_H
#include "OperatorTable.h"
#include "OpMemoryTable.h"
namespace Dic::Module::Memory {
struct OperatorDomain {
    std::string metaType;
    uint64_t allocationTime = 0;
};
class OperatorMemoryService {
public:
    explicit OperatorMemoryService(std::shared_ptr<OperatorTable> operatorTablePtr = std::make_shared<OperatorTable>(),
        std::shared_ptr<OpMemoryTable> opMemoryTablePtr = std::make_shared<OpMemoryTable>());
    OperatorDomain ComputeAllocationTimeById(const std::string &rankId, const std::string &id);

private:
    std::shared_ptr<OperatorTable> operatorTable = nullptr;
    std::shared_ptr<OpMemoryTable> opMemoryTable = nullptr;
};
}
#endif // PROFILER_SERVER_OPERATORMEMORYSERVICE_H
