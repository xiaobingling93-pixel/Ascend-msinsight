/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_NPUINFOREPOMOCK_H
#define PROFILER_SERVER_NPUINFOREPOMOCK_H
#include "NpuInfoRepo.h"
#include "TraceDatabaseHelper.h"

class NpuInfoRepoMock : public Dic::Protocol::NpuInfoRepo {
public:
    std::vector<uint64_t> QueryDeviceIdByFileId(const std::string &fileId) override
    {
        std::vector<uint64_t> res;
        res.push_back(0);
        return res;
    }
};
inline void MockNpuInfoRepoFunc()
{
    std::unique_ptr<Dic::Protocol::NpuInfoRepo> tPtr = std::make_unique<NpuInfoRepoMock>();
    Dic::Protocol::TraceDatabaseHelper::SetNpuInfoRepo(std::move(tPtr));
}

inline void RestoreRepoFunc()
{
    std::unique_ptr<Dic::Protocol::NpuInfoRepo> tPtr = std::make_unique<Dic::Protocol::NpuInfoRepo>();
    Dic::Protocol::TraceDatabaseHelper::SetNpuInfoRepo(std::move(tPtr));
}

#endif // PROFILER_SERVER_NPUINFOREPOMOCK_H
