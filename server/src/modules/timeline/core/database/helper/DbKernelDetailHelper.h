/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef DBKERNELDETAILHELPER_H
#define DBKERNELDETAILHELPER_H

#include "TimelineProtocolRequest.h"
#include "VirtualTraceDatabase.h"

namespace Dic::Module::FullDb {
using namespace Dic::Module::Timeline;
class DbKernelDetailHelper {
public:
    static std::string GetKernelDetailSql(const Protocol::KernelDetailsParams &requestParams, bool isLowCamel);
private:
    static std::string GetKernelDetailSqlWithHCCL(const Protocol::KernelDetailsParams &requestParams);
    static std::string GetKernelDetailSqlWithoutHCCL(const Protocol::KernelDetailsParams &requestParams,
        bool isLowCamel);
    static std::string GetKernelDetailFilterSqlWithHCCL(const Protocol::KernelDetailsParams &requestParams);
    static std::string GetKernelDetailFilterSqlWithoutHCCL(const Protocol::KernelDetailsParams &requestParams);
};
} // namespace Dic::Module::FullDb

#endif // DBKERNELDETAILHELPER_H
