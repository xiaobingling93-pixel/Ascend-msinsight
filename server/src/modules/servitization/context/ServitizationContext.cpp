/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "IEContext.h"
#include "ServitizationContext.h"
namespace Dic::Module::IE {
bool ServitizationContext::InitDataBase(const std::string& fileId, const std::string& dbPath)
{
    return IEContext::Instance().InitDataBase(fileId, dbPath);
}

std::string ServitizationContext::ComputeFileIdByFolder(const std::string& folder)
{
    return IEContext::Instance().ComputeFileIdByFolder(folder);
}

bool ServitizationContext::ExecuteScript(const std::string& fileId, const std::string& script)
{
    return IEContext::Instance().ExecSql(fileId, script);
}

std::shared_ptr<Database> ServitizationContext::GetDatabase(const std::string& fileId)
{
    return IEContext::Instance().GetDatabase(fileId);
}

void ServitizationContext::Reset()
{
    IEContext::Instance().Reset();
}
}  // namespace Dic::Module::IE