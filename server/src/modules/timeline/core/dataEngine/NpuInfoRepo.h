/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_NPUINFOREPO_H
#define PROFILER_SERVER_NPUINFOREPO_H
#include <vector>
#include "string"
#include "NpuInfoTable.h"
#include "MstxEventsTable.h"
namespace Dic::Module::Timeline {
class NpuInfoRepo {
public:
    virtual std::vector<uint64_t> QueryDeviceIdByFileId(const std::string &fileId);
    void SetNpuInfoTable(std::unique_ptr<NpuInfoTable> npuInfoTablePtr);

protected:
    std::unique_ptr<NpuInfoTable> npuInfoTable = std::make_unique<NpuInfoTable>();
};
}
#endif // PROFILER_SERVER_NPUINFOREPO_H
