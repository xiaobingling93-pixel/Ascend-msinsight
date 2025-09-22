/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "ConnectionCatsParseUnit.h"
#include "ConstantDefs.h"
#include "ParseUnitManager.h"
#include "TableDefs.h"

namespace Dic::Module::FullDb {
    std::string ConnectionCatsParseUnit::GetUnitName()
    {
        return CONNECTION_UNIT;
    }

    bool ConnectionCatsParseUnit::PreCheck(const ParseUnitParams &params,
                                           const std::shared_ptr<DbTraceDataBase> &database, std::string &error)
    {
        return true;
    }

    bool ConnectionCatsParseUnit::HandleParseProcess(const ParseUnitParams &params,
                                                     const std::shared_ptr<DbTraceDataBase> &database,
                                                     std::string &error)
    {
        bool res = database->InitConnectionCats();
        if (!res) {
            error = "Fail to init connection category.";
        }
        return res;
    }

    ParseUnitRegistrar<ConnectionCatsParseUnit> unitRegConnection(CONNECTION_UNIT);
}