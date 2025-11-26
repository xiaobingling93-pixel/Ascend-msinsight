/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ServerLog.h"
#include "DataBaseManager.h"
#include "QueryMemScopeBlockHandler.h"

namespace Dic::Module::MemScope {
enum class SimpleBlockEventType {
    MALLOC = 0,
    FREE = 1
};
struct SimpleBlockEvent {
    SimpleBlockEventType type;
    uint64_t timestamp;
    std::shared_ptr<MemoryBlockItem> blockItemPtr;

    SimpleBlockEvent(const SimpleBlockEventType type, const uint64_t timestamp,
                     std::shared_ptr<MemoryBlockItem> blockItemPtr)
        : type(type), timestamp(timestamp), blockItemPtr(std::move(blockItemPtr)) {}
};
bool QueryMemScopeBlockHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<MemScopeMemoryBlockRequest &>(*requestPtr);
    std::unique_ptr<MemScopeMemoryBlocksResponse> responsePtr = std::make_unique<MemScopeMemoryBlocksResponse>();
    auto &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    bool success = request.isTable ? HandleBlocksTableRequest(request, response, errorMsg) :
                                     HandleBlocksViewRequest(request, response, errorMsg);
    SendResponse(std::move(responsePtr), success, errorMsg);
    return success;
}
static bool CompareEvent(const std::shared_ptr<SimpleBlockEvent>& base,
                         const std::shared_ptr<SimpleBlockEvent>& compare)
{
    if (base->timestamp == compare->timestamp) {
        return base->type < compare->type;
    }
    return base->timestamp < compare->timestamp;
}

static void SplitMemoryBlocks2Events(const std::vector<MemoryBlock> &blocks,
                                     std::vector<std::shared_ptr<SimpleBlockEvent>> &sortedEvents,
                                     std::vector<std::shared_ptr<MemoryBlockItem>> &blockItems)
{
    for (auto &block : blocks) {
        // 每个block构成两个事件：分配、释放
        auto blockItemPtr = std::make_shared<MemoryBlockItem>(block);
        blockItems.push_back(blockItemPtr);
        sortedEvents.emplace_back(std::make_shared<SimpleBlockEvent>
                                          (SimpleBlockEventType::MALLOC, block.startTimestamp, blockItemPtr));
        sortedEvents.emplace_back(std::make_shared<SimpleBlockEvent>
                                          (SimpleBlockEventType::FREE, block.endTimestamp, blockItemPtr));
    }
    // 排序
    std::sort(sortedEvents.begin(), sortedEvents.end(), CompareEvent);
}

void BuildBlocksResponseBySortedEvent(std::vector<std::shared_ptr<SimpleBlockEvent>> &sortedEvents,
                                      MemScopeMemoryBlocksResponse &response)
{
    if (sortedEvents.empty()) {
        Server::ServerLog::Error("Empty sorted events");
        return;
    }
    std::stack<std::shared_ptr<MemoryBlockItem>> currentBlockStack;
    std::stack<std::shared_ptr<MemoryBlockItem>> tempBlockStack;
    uint64_t currentTotalSize = 0;
    response.maxTimestamp = sortedEvents.back()->timestamp;
    response.minTimestamp = sortedEvents[0]->timestamp;
    for (size_t idx = 0; idx < sortedEvents.size(); idx++) {
        auto eventPtr = sortedEvents[idx];
        if (eventPtr->type == SimpleBlockEventType::MALLOC) {
            currentBlockStack.push(eventPtr->blockItemPtr);
            eventPtr->blockItemPtr->AddPathPoint(eventPtr->timestamp, currentTotalSize);
            currentTotalSize = NumberSafe::Add(currentTotalSize, eventPtr->blockItemPtr->size);
            response.maxSize = std::max(response.maxSize, currentTotalSize);
            continue;
        }
        while (!currentBlockStack.empty()) {
            std::shared_ptr<MemoryBlockItem> tempItemPtr = currentBlockStack.top();
            currentBlockStack.pop();
            currentTotalSize = NumberSafe::Sub(currentTotalSize, tempItemPtr->size);
            tempItemPtr->AddPathPoint(eventPtr->timestamp, currentTotalSize);
            if (tempItemPtr->id != eventPtr->blockItemPtr->id) {
                tempBlockStack.push(tempItemPtr);
            }
        }
        while (!tempBlockStack.empty()) {
            std::shared_ptr<MemoryBlockItem> tempItemPtr = tempBlockStack.top();
            tempBlockStack.pop();
            currentBlockStack.push(tempItemPtr);
            // 此处忽略截断的精度丢失
            uint64_t  tmpTimestamp = idx + 1 < sortedEvents.size() ?
                    (eventPtr->timestamp + sortedEvents[idx+1]->timestamp)/2 : response.maxTimestamp;
            tempItemPtr->AddPathPoint(tmpTimestamp, currentTotalSize);
            currentTotalSize = NumberSafe::Add(currentTotalSize, tempItemPtr->size);
        }
    }
    // 处理剩余块
    while (!currentBlockStack.empty()) {
        std::shared_ptr<MemoryBlockItem> tempItemPtr = currentBlockStack.top();
        currentBlockStack.pop();
        tempItemPtr->AddPathPoint(response.maxTimestamp, currentTotalSize);
        currentTotalSize = NumberSafe::Sub(currentTotalSize, tempItemPtr->size);
    }
    response.total = response.blocks.size();
}

void QueryMemScopeBlockHandler::BuildBlocksViewResponse(const std::vector<MemoryBlock> &blocks,
                                                        MemScopeMemoryBlocksResponse &response)
{
    if (blocks.empty()) {
        Server::ServerLog::Warn("Empty blocks.");
        return;
    }
    std::vector<std::shared_ptr<SimpleBlockEvent>> sortedEvents;
    std::vector<std::shared_ptr<MemoryBlockItem>> blockItemPtrList;
    SplitMemoryBlocks2Events(blocks, sortedEvents, blockItemPtrList);
    BuildBlocksResponseBySortedEvent(sortedEvents, response);
    response.blocks.assign(blockItemPtrList.begin(), blockItemPtrList.end());
    response.withPath = true;
}

bool QueryMemScopeBlockHandler::HandleBlocksTableRequest(MemScopeMemoryBlockRequest& request,
                                                         MemScopeMemoryBlocksResponse& response,
                                                         std::string &errorMsg)
{
    auto memoryDatabase = Timeline::DataBaseManager::Instance().GetMemScopeDatabase("");
    if (memoryDatabase == nullptr) {
        errorMsg = "Failed to query memory blocks: get database connection failed.";
        return false;
    }
    std::vector<MemoryBlock> blocks;
    int64_t total = memoryDatabase->QueryMemoryBlocks(request.params, request.isTable, blocks);
    if (total < 0) {
        errorMsg = "Failed to query memory blocks: query db failed.";
        return false;
    }
    response.total = static_cast<uint64_t>(total);
    std::transform(blocks.begin(), blocks.end(), std::back_inserter(response.blocks),
                   [](const MemoryBlock& block) {
                       return std::make_shared<MemoryBlock>(block);
                   });
    return true;
}
bool QueryMemScopeBlockHandler::HandleBlocksViewRequest(MemScopeMemoryBlockRequest& request,
                                                        MemScopeMemoryBlocksResponse& response,
                                                        std::string &errorMsg)
{
    auto memoryDatabase = Timeline::DataBaseManager::Instance().GetMemScopeDatabase("");
    if (memoryDatabase == nullptr) {
        errorMsg = "Failed to query memory blocks: get database connection failed.";
        return false;
    }
    std::vector<MemoryBlock> blocks;
    memoryDatabase->QueryMemoryBlocks(request.params, request.isTable, blocks);
    if (blocks.empty()) {
        Server::ServerLog::Warn("Query memory blocks: empty data.");
        return true;
    }
    BuildBlocksViewResponse(blocks, response);
    return true;
}
} // Dic::Module::MemScope