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

#ifndef PROFILER_SERVER_MEM_SNAPSHOT_PROTOCOL_RESPONSE_H
#define PROFILER_SERVER_MEM_SNAPSHOT_PROTOCOL_RESPONSE_H
#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
#include "CommonRequests.h"
#include "MemSnapshotDefs.h"
#include "MemSnapshotTableColumn.h"

namespace Dic::Protocol {
using namespace Dic::Module;
using namespace Dic::Module::MemSnapshot;

static document_t ToMemSnapshotBlockViewJson(const Block& block,
                                             Document::AllocatorType& allocator,
                                             const uint64_t maxTimestamp)
{
    document_t json(kObjectType);
    JsonUtil::AddMember(json, "id", block.id, allocator);
    JsonUtil::AddMember(json, "addr", std::to_string(block.address), allocator);
    JsonUtil::AddMember(json, "size", block.size, allocator);
    JsonUtil::AddMember(json, "_startTimestamp", block.allocEventId < 0 ? 0 : block.allocEventId, allocator);
    JsonUtil::AddMember(json, "_endTimestamp", block.freeEventId < 0 ? maxTimestamp : block.freeEventId, allocator);
    return json;
}

static document_t ToMemSnapshotBlockTableJson(const Block& block,
                                              Document::AllocatorType& allocator)
{
    document_t json(kObjectType);
    JsonUtil::AddMember(json, "id", block.id, allocator);
    JsonUtil::AddMember(json, "address", std::to_string(block.address), allocator);
    JsonUtil::AddMember(json, "size", block.size, allocator);
    JsonUtil::AddMember(json, "requestedSize", block.requestedSize, allocator);
    JsonUtil::AddMember(json, "state", block.state, allocator);
    JsonUtil::AddMember(json, "allocEventId", block.allocEventId, allocator);
    JsonUtil::AddMember(json, "freeEventId", block.freeEventId, allocator);
    return json;
}

struct MemSnapshotBlocksResponse : public JsonResponse {
    MemSnapshotBlocksResponse() : JsonResponse(REQ_RES_MEM_SNAPSHOT_BLOCKS) {}
    std::vector<Block> blocks;
    uint64_t minSize{0};
    uint64_t maxSize{0};
    uint64_t minTimestamp{0};
    uint64_t maxTimestamp{0};
    uint64_t total{0};
    bool isTable{false};

    [[nodiscard]] std::optional<document_t> ToJson() const override
    {
        document_t json(kObjectType);
        auto& allocator = json.GetAllocator();
        ProtocolUtil::SetResponseJsonBaseInfo(*this, json);
        json_t body(kObjectType);
        json_t jsonBlocks(kArrayType);
        JsonUtil::AddMember(body, "minTimestamp", minTimestamp, allocator);
        JsonUtil::AddMember(body, "maxTimestamp", maxTimestamp, allocator);
        JsonUtil::AddMember(body, "minSize", minSize, allocator);
        JsonUtil::AddMember(body, "maxSize", maxSize, allocator);
        JsonUtil::AddMember(body, "total", total, allocator);
        json_t jsonHeaders(kArrayType);
        for (const auto& header : BlockTableColumn::FIELD_FULL_COLUMNS) {
            if (header.visible) { jsonHeaders.PushBack(header.ToTableHeaderJson(allocator), allocator); }
        }
        if (isTable) {
            for (const auto& block : blocks) {
                auto blockJson = ToMemSnapshotBlockTableJson(block, allocator);
                jsonBlocks.PushBack(blockJson, allocator);
            }
        } else {
            for (const auto& block : blocks) {
                auto blockJson = ToMemSnapshotBlockViewJson(block, allocator, maxTimestamp);
                jsonBlocks.PushBack(blockJson, allocator);
            }
        }
        JsonUtil::AddMember(body, "blocks", jsonBlocks, allocator);
        JsonUtil::AddMember(body, "headers", jsonHeaders, allocator);
        JsonUtil::AddMember(json, "body", body, allocator);
        return std::optional<document_t>{std::move(json)};
    }
};

static document_t ToMemSnapshotTraceEntryJson(const TraceEntry& entry,
                                              Document::AllocatorType& allocator,
                                              const bool isTable = false)
{
    document_t json(kObjectType);
    JsonUtil::AddMember(json, "id", entry.id, allocator);
    JsonUtil::AddMember(json, "action", entry.action, allocator);
    JsonUtil::AddMember(json, "address", entry.address, allocator);
    JsonUtil::AddMember(json, "size", entry.size, allocator);
    JsonUtil::AddMember(json, "stream", entry.stream, allocator);
    if (isTable) {
        JsonUtil::AddMember(json, "allocated", entry.allocated, allocator);
        JsonUtil::AddMember(json, "active", entry.active, allocator);
        JsonUtil::AddMember(json, "reserved", entry.reserved, allocator);
        JsonUtil::AddMember(json, "callstack", entry.callstack, allocator);
    }
    return json;
}

struct MemSnapshotEventsResponse : public JsonResponse {
    MemSnapshotEventsResponse() : JsonResponse(REQ_RES_MEM_SNAPSHOT_EVENTS) {}
    std::vector<TraceEntry> entries;
    uint64_t total{0};
    uint64_t minTimestamp{0};
    uint64_t maxTimestamp{0};

    bool isTable{false};

    [[nodiscard]] std::optional<document_t> ToJson() const override
    {
        document_t json(kObjectType);
        auto& allocator = json.GetAllocator();
        ProtocolUtil::SetResponseJsonBaseInfo(*this, json);
        json_t body(kObjectType);
        json_t jsonEntries(kArrayType);
        JsonUtil::AddMember(body, "minTimestamp", minTimestamp, allocator);
        JsonUtil::AddMember(body, "maxTimestamp", maxTimestamp, allocator);
        JsonUtil::AddMember(body, "total", total, allocator);
        json_t jsonHeaders(kArrayType);
        for (const auto& header : TraceEntryTableColumn::FIELD_FULL_COLUMNS) {
            if (header.visible) { jsonHeaders.PushBack(header.ToTableHeaderJson(allocator), allocator); }
        }
        for (const auto& entry : entries) {
            auto entryJson = ToMemSnapshotTraceEntryJson(entry, allocator, isTable);
            jsonEntries.PushBack(entryJson, allocator);
        }
        JsonUtil::AddMember(body, "events", jsonEntries, allocator);
        JsonUtil::AddMember(body, "headers", jsonHeaders, allocator);
        JsonUtil::AddMember(json, "body", body, allocator);
        return std::optional<document_t>{std::move(json)};
    }
};

struct MemSnapshotAllocationsResponse : public JsonResponse {
    MemSnapshotAllocationsResponse() : JsonResponse(REQ_RES_MEM_SNAPSHOT_ALLOCATIONS) {}
    uint64_t minEventId{0};
    uint64_t maxEventId{0};
    std::vector<MemoryRecord> records;

    [[nodiscard]] std::optional<document_t> ToJson() const override
    {
        document_t json(kObjectType);
        auto& allocator = json.GetAllocator();
        ProtocolUtil::SetResponseJsonBaseInfo(*this, json);
        json_t body(kObjectType);
        json_t recordsJson(kArrayType);
        JsonUtil::AddMember(body, "minTimestamp", minEventId, allocator);
        JsonUtil::AddMember(body, "maxTimestamp", maxEventId, allocator);
        for (const auto& record : records) {
            auto recordJson = ToMemSnapshotMemoryRecordJson(record, allocator);
            if (recordJson.has_value()) {
                recordsJson.PushBack(recordJson.value(), allocator);
            }
        }
        JsonUtil::AddMember(body, "allocations", recordsJson, allocator);
        JsonUtil::AddMember(json, "body", body, allocator);
        return std::optional<document_t>{std::move(json)};
    }

    static std::optional<document_t> ToMemSnapshotMemoryRecordJson(const MemoryRecord& record,
                                                                  Document::AllocatorType& allocator)
    {
        document_t json(kObjectType);
        JsonUtil::AddMember(json, "id", record.id, allocator);
        JsonUtil::AddMember(json, "timestamp", record.id, allocator);
        JsonUtil::AddMember(json, "totalSize", record.allocated, allocator);
        return std::optional<document_t>{std::move(json)};
    }
};
}
#endif  // PROFILER_SERVER_MEM_SNAPSHOT_PROTOCOL_RESPONSE_H
