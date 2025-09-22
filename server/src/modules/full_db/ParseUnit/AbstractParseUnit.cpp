/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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
