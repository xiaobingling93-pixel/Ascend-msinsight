/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ParseUnitManager.h"
#include "ConstantDefs.h"
#include "ServerLog.h"
namespace Dic::Module::FullDb {
    ParseUnitManager &ParseUnitManager::Instance()
    {
        static ParseUnitManager instance;
        return instance;
    }

    void ParseUnitManager::RegisterUnit(const std::string& name, Creator unit)
    {
        unitMap[name] = unit;
    }

    void ParseUnitManager::ExecuteAll(const ParseUnitParams &params)
    {
        for (const auto &item: unitMap) {
            Server::ServerLog::Info("Start execute parse unit, unit name:", item.first);
            bool res = item.second()->Handle(params);
            Server::ServerLog::Info("End execute parse unit, unit name:", item.first, ", result:", res);
        }
    }
}
