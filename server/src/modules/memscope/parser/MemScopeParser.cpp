/*
* -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

#include "pch.h"
#include "DataBaseManager.h"
#include "MemScopeProtocolEvent.h"
#include "MemScopeService.h"
#include "MemScopeParser.h"

namespace Dic::Module {
MemScopeParser& MemScopeParser::Instance()
{
    static MemScopeParser instance;
    return instance;
}

void MemScopeParser::Reset() const
{
    FullDb::MemScopeDatabase::Reset();
    _threadPool->Reset();
}

void MemScopeParser::AsyncParseMemScopeDbFile(const std::string& dbPath) const
{
    _threadPool->AddTask(ParseMemScopeDbTask, TraceIdManager::GetTraceId(), dbPath);
}

void MemScopeParser::ParseMemScopeDbTask(const std::string& dbPath)
{
    auto database = Timeline::DataBaseManager::Instance().GetMemScopeDatabase(dbPath);
    if (database == nullptr || !database->OpenDb(dbPath, false)) {
        const std::string err = "Failed to get memscope database";
        Server::ServerLog::Error(err);
        ParserEnd(dbPath, false);
        ParseCallBack(dbPath, false, err);
        return;
    }
    if (!database->CheckTableExist(TABLE_LEAKS_DUMP) && !database->CheckTableExist(TABLE_MEM_SCOPE_DUMP)) {
        const std::string err = "The 'leaks_dump' table or 'memscope_dump' table should exist in the memscope "
            "database at a minimum.";
        Server::ServerLog::Error(err);
        ParserEnd(dbPath, false);
        ParseCallBack(dbPath, false, err);
        return;
    }
    if (MemScopeService::ParseMemoryMemScopeDumpEventsAndPythonTraces(dbPath)) {
        ParserEnd(dbPath, true);
        ParseCallBack(dbPath, true, "");
    }
    else {
        Server::ServerLog::Error("Failed to connect or open memscope memory database.");
        ParserEnd(dbPath, false);
        ParseCallBack(dbPath, false,
                      "An exception occurred while parsing the DB data: "
                      "Please check the logs for details.");
    }
    Timeline::ParserStatusManager::Instance().SetParserStatus(dbPath, Timeline::ParserStatus::FINISH_ALL);
}

void MemScopeParser::ParserEnd(const std::string& dbPath, const bool result)
{
    if (!result) {
        Server::ServerLog::Error("[MemScope]memscope database parser failed, filepath: ", dbPath);
        return;
    }
    Server::ServerLog::Info("[MemScope]memscope Dumps Parser ends, filepath: ", dbPath);
}

void MemScopeParser::ParseCallBack(const std::string& dbPath, bool result, const std::string& msg)
{
    auto event = std::make_unique<Protocol::MemScopeParseSuccessEvent>();
    event->moduleName = Protocol::MODULE_MEM_SCOPE;
    if (dbPath.empty()) {
        event->result = true;
        SendEvent(std::move(event));
    }
    else {
        event->result = result;
        Protocol::MemScopeParseSuccessEventBody body;
        if (event->result) {
            const auto memoryDatabase = Timeline::DataBaseManager::Instance().GetMemScopeDatabase("");
            if (memoryDatabase == nullptr) {
                Server::ServerLog::Error("Cannot get memscope db connections from database manager");
                event->errMsg = "Failed parse memscope dump data.";
                event->result = false;
                SendEvent(std::move(event));
                return;
            }
            memoryDatabase->QueryMallocOrFreeEventTypeWithDeviceId(body.deviceIds);
            memoryDatabase->QueryThreadIds(body.threadIds);
            memoryDatabase->SetDataBaseVersion();
        }
        else { event->errMsg = msg; }
        body.fileId = dbPath;
        event->body = body;
        SendEvent(std::move(event));
    }
}

MemScopeParser::MemScopeParser()
{
    // MemScopedb为单文件单线程解析
    _threadPool = std::make_unique<ThreadPool>(1);
}

MemScopeParser::~MemScopeParser() { if (_threadPool != nullptr) { _threadPool->ShutDown(); } }
}
