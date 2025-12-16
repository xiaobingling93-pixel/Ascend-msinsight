/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
#include "AbstractParseUnit.h"
#include <thread>
#include "DataBaseManager.h"

namespace Dic::Module::FullDb {
bool AbstractParseUnit::Handle(const ParseUnitParams &params)
{
    std::string error;
    bool res = Parse(params, error);
    SendNotify(params, res, error);
    if (!error.empty()) {
        ServerLog::Warn("Execute parse unit failed, error:", error);
    }
    return res;
}

bool AbstractParseUnit::Parse(const ParseUnitParams &params, std::string &error)
{
    // get unit name
    std::string unitName = GetUnitName();
    auto db = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId(params.dbId);
    std::shared_ptr<DbTraceDataBase> database =
        std::dynamic_pointer_cast<DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
    if (!database) {
        error = StringUtil::FormatString("Failed to parse unit, the database is not exist. Unit name:{}", unitName);
        return false;
    }
    // check unit status
    if (database->CheckValueFromStatusInfoTable(unitName, FINISH_STATUS)) { // 已更新数据，跳过更新
        ServerLog::Info("The parser unit status is completed, skip paring, unit name:", unitName);
        return true;
    }
    // pre check
    if (!PreCheck(params, database, error)) {
        return false;
    }
    // Handle unit parse work
    if (!HandleParseProcess(params, database, error)) {
        return false;
    }

    // insert or update status
    database->UpdateValueIntoStatusInfoTable(unitName, FINISH_STATUS);
    return true;
}

void AbstractParseUnit::SendNotify(const ParseUnitParams &params, bool parseRes, const std::string &error)
{
    auto event = std::make_unique<ParseUnitCompletedEvent>();
    event->body.parseResult = parseRes;
    event->body.unitName = GetUnitName();
    event->body.errorMsg = error;
    event->body.dbId = params.dbId;
    event->moduleName = MODULE_TIMELINE;
    event->result = true;
    SendEvent(std::move(event));
}
}
