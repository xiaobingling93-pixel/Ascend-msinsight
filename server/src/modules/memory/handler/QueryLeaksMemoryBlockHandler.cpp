/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ServerLog.h"
#include "DataBaseManager.h"
#include "MemoryProtocolRespose.h"
#include "MemoryProtocolRequest.h"
#include "QueryLeaksMemoryBlockHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
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
bool QueryLeaksMemoryBlockHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<LeaksMemoryBlockRequest &>(*requestPtr);
    std::unique_ptr<LeaksMemoryBlocksResponse> responsePtr = std::make_unique<LeaksMemoryBlocksResponse>();
    auto &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto memoryDatabase = Timeline::DataBaseManager::Instance().GetLeaksMemoryDatabase("");
    if (memoryDatabase == nullptr) {
        Server::ServerLog::Error("Failed to query memory blocks: get database connection failed.");
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    std::vector<MemoryBlock> blocks;
    memoryDatabase->QueryMemoryBlocks(request.params, blocks);
    if (blocks.empty()) {
        Server::ServerLog::Warn("Query memory blocks: empty data.");
        SendResponse(std::move(responsePtr), true);
        return true;
    }
    BuildBlocksResponse(blocks, response);
    SendResponse(std::move(responsePtr), true);
    return true;
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
                                      LeaksMemoryBlocksResponse &response)
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
    response.maxSize = 0;
    response.minSize = 0;

    for (auto &eventPtr : sortedEvents) {
        if (eventPtr->type == SimpleBlockEventType::MALLOC) {
            currentBlockStack.push(eventPtr->blockItemPtr);
            eventPtr->blockItemPtr->path.emplace_back(eventPtr->timestamp, currentTotalSize);
            currentTotalSize += eventPtr->blockItemPtr->size;
            response.maxSize = std::max(response.maxSize, currentTotalSize);
            continue;
        }
        while (!currentBlockStack.empty()) {
            std::shared_ptr<MemoryBlockItem> tempItemPtr = currentBlockStack.top();
            currentBlockStack.pop();
            currentTotalSize -= tempItemPtr->size;
            if (tempItemPtr->id == eventPtr->blockItemPtr->id) {
                tempItemPtr->path.emplace_back(eventPtr->timestamp, currentTotalSize);
                break;
            }
            tempBlockStack.push(tempItemPtr);
        }
        while (!tempBlockStack.empty()) {
            std::shared_ptr<MemoryBlockItem> tempItemPtr = tempBlockStack.top();
            tempBlockStack.pop();
            currentBlockStack.push(tempItemPtr);
            tempItemPtr->path.emplace_back(eventPtr->timestamp, currentTotalSize);
            currentTotalSize += tempItemPtr->size;
        }
    }
    // 处理剩余块
    while (!currentBlockStack.empty()) {
        std::shared_ptr<MemoryBlockItem> tempItemPtr = currentBlockStack.top();
        currentBlockStack.pop();
        tempItemPtr->path.emplace_back(response.maxTimestamp, currentTotalSize);
        currentTotalSize -= tempItemPtr->size;
    }
    response.totalNum = response.blocks.size();
}

void QueryLeaksMemoryBlockHandler::BuildBlocksResponse(const std::vector<MemoryBlock> &blocks,
                                                       LeaksMemoryBlocksResponse &response)
{
    if (blocks.empty()) {
        Server::ServerLog::Warn("Empty blocks.");
        return;
    }
    std::vector<std::shared_ptr<SimpleBlockEvent>> sortedEvents;
    std::vector<std::shared_ptr<MemoryBlockItem>> blockItemPtrList;
    SplitMemoryBlocks2Events(blocks, sortedEvents, blockItemPtrList);
    BuildBlocksResponseBySortedEvent(sortedEvents, response);
    for (auto &blockItemPtr : blockItemPtrList) {
        response.blocks.push_back(*blockItemPtr);
    }
}
} // Memory
} // Module
} // Dic