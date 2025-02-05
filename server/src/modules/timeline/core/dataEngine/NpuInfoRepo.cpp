/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "NpuInfoRepo.h"

namespace Dic::Module::Timeline {
    std::vector<uint64_t> NpuInfoRepo::QueryDeviceIdByFileId(const std::string &fileId)
    {
        std::vector<NpuInfoPo> npuInfoPos = npuInfoTable->Select(NpuInfoColumn::ID)
                .GroupBy(NpuInfoColumn::ID)
                .ExcuteQuery(fileId);
        std::vector<uint64_t> deviceIdList;
        for (const auto &item: npuInfoPos) {
            deviceIdList.push_back(item.id);
        }
        return deviceIdList;
    }

    void NpuInfoRepo::SetNpuInfoTable(std::unique_ptr<NpuInfoTable> npuInfoTablePtr)
    {
        if (npuInfoTablePtr != nullptr) {
            npuInfoTable = std::move(npuInfoTablePtr);
        }
    }
}