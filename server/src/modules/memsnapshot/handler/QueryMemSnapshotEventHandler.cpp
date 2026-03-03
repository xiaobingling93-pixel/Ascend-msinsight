/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "QueryMemSnapshotEventHandler.h"
#include "DataBaseManager.h"

using namespace Dic::Module::MemSnapshot;

namespace Dic::Module::MemSnapshot {
bool QueryMemSnapshotEventHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto& request = dynamic_cast<MemSnapshotEventsRequest&>(*requestPtr);
    std::unique_ptr<MemSnapshotEventsResponse> responsePtr = std::make_unique<MemSnapshotEventsResponse>();
    auto& response = *responsePtr;
    response.isTable = request.isTable;
    SetBaseResponse(request, response);
    std::string errMsg;
    if (!request.params.CommonCheck(errMsg)) {
        SendResponse(std::move(responsePtr), false, errMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetMemSnapshotDatabase(request.projectName);
    if (database == nullptr || !database->IsOpen()) {
        errMsg = LOG_TAG + "Failed to query events: get database connection failed";
        SendResponse(std::move(responsePtr), false, errMsg);
        return false;
    }
    const int64_t total = database->QueryTraceEntriesTable(request.params, response.entries);
    if (total < 0) {
        errMsg = LOG_TAG + "Failed to query events: query db failed.";
        SendResponse(std::move(responsePtr), false, errMsg);
        return false;
    }
    response.total = static_cast<uint64_t>(total);
    response.maxTimestamp = database->QueryMaxEntryId();
    SendResponse(std::move(responsePtr), true);
    return true;
}
} // namespace Dic::Module::MemSnapshot
