/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVITIZATIONCONTEXT_H
#define PROFILER_SERVER_SERVITIZATIONCONTEXT_H
#include <string>
#include "Database.h"
namespace Dic::Module::IE {
class ServitizationContext {
public:
    virtual bool InitDataBase(const std::string& fileId, const std::string& dbPath);
    virtual std::string ComputeFileIdByFolder(const std::string& folder);
    virtual bool ExecuteScript(const std::string& fileId, const std::string& script);
    virtual std::shared_ptr<Database> GetDatabase(const std::string& fileId);
    /* *
         * 清理所有上下文
         */
    virtual void Reset();
};
}  // namespace Dic::Module::IE
#endif  // PROFILER_SERVER_SERVITIZATIONCONTEXT_H
