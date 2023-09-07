/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_DATABASE_H
#define PROFILER_SERVER_COMMUNICATION_DATABASE_H

#include "Database.h"
#include "Protocol.h"
#include "SummaryDef.h"

namespace Dic {
namespace Module {
namespace Summary {
class SummaryDataBase : public Database {
public:
    SummaryDataBase() = default;
    ~SummaryDataBase() override;

    bool SetConfig();
    bool CreateTable();
    bool InitStmt();
    void ReleaseStmt();

    bool InsertKernelDetailList(std::vector<Kernel> kernelVec);
    void InsertKernelDetail(Kernel kernel);
    void SaveKernelDetail();

private:
    const std::string kernelTable = "kernel_detail";
    bool hasInitStmt = false;
    sqlite3_stmt *insertKernelStmt = nullptr;
    const int cacheSize = 1000;
    std::vector<Kernel> kernelCache;

    sqlite3_stmt *GetKernelStmt(uint64_t paramLen);
};

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_SUMMARY_DATABASE_H
