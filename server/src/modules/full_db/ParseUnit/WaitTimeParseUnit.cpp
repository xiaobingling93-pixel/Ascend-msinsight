/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "WaitTimeParseUnit.h"
#include "ConstantDefs.h"
#include "ParseUnitManager.h"
#include "TableDefs.h"

namespace Dic::Module::FullDb {
    std::string WaitTimeParseUnit::GetUnitName()
    {
        return WAIT_TIME_UNIT;
    }

    bool WaitTimeParseUnit::PreCheck(const ParseUnitParams &params, const std::shared_ptr<DbTraceDataBase> &database,
                                     std::string &error)
    {
        bool checkRes = database->CheckTableExist(TABLE_COMPUTE_TASK_INFO)
            && database->CheckTableExist(TABLE_COMMUNICATION_OP)
            && database->CheckTableDataInvalid(TABLE_TASK);
        if (!checkRes) {
            error = "Update wait time:Table is not exist or table data invalid.";
        }
        return checkRes;
    }

    bool WaitTimeParseUnit::HandleParseProcess(const ParseUnitParams &params,
                                               const std::shared_ptr<DbTraceDataBase> &database, std::string &error)
    {
        database->UpdateWaitTime();
        return true;
    }

    ParseUnitRegistrar<WaitTimeParseUnit> unitRegWaitTime(WAIT_TIME_UNIT);
}
