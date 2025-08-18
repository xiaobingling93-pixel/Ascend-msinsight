/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
#include "MemoryDetailEntities.h"
namespace Dic::Module::MemoryDetail {
// [PYTHON_TRACE]
static uint SMALL_RATIO = 10000;
static std::string DEFAULT_MERGED_SEP = " -> ";
bool LeaksMemoryPythonTrace::Empty() const
{
    return slices.empty();
}

void LeaksMemoryPythonTrace::Trim(const PythonTrimCompressStrategy& strategy)
{
    if (maxTimestamp <= minTimestamp) {
        Server::ServerLog::Warn("Failed to compress python trace, the minTimestamp is greater than maxTimestamp.");
        return;
    }
    bool needCompress = strategy == PythonTrimCompressStrategy::COMPRESS_SMALL_FUNCTIONS ||
                        strategy == PythonTrimCompressStrategy::COMPRESS_AND_FILTER_SMALL_FUNCTIONS;
    bool needFilterOut = strategy == PythonTrimCompressStrategy::ONLY_FILTER_OUT_SMALL_FUNCTIONS ||
                         strategy == PythonTrimCompressStrategy::COMPRESS_AND_FILTER_SMALL_FUNCTIONS;
    // 先筛选出small slices
    std::vector<PythonTraceSlice> smallSlices;
    auto new_end =
        std::remove_if(slices.begin(), slices.end(), [this, &smallSlices, needCompress](const PythonTraceSlice& slice) {
            if (IsSmallSlice(slice)) {
                // 如果需要压缩, 缓存到smallSlices数组
                if (needCompress) {
                    smallSlices.push_back(slice);
                }
                return true;
            }
            return false;
        });
    slices.erase(new_end, slices.end());
    if (needCompress) {
        DoCompress(smallSlices, needFilterOut);
    }
}

bool LeaksMemoryPythonTrace::IsSmallSlice(const PythonTraceSlice& slice) const
{
    if (slice.endTimestamp < slice.startTimestamp) {
        return true;
    }

    uint64_t duration = slice.endTimestamp - slice.startTimestamp;
    return duration  < (maxTimestamp - minTimestamp) / SMALL_RATIO;
}

void LeaksMemoryPythonTrace::DoCompress(std::vector<PythonTraceSlice> &waitForCompressSlices, const bool filterOutSmallFunc)
{
    // 1. 按照开始时间进行排序
    std::sort(waitForCompressSlices.begin(), waitForCompressSlices.end(), [](const PythonTraceSlice &a, const PythonTraceSlice &b) {
        return a.startTimestamp < b.startTimestamp;
    });
    // 2. 按照深度分组
    std::vector<std::vector<PythonTraceSlice>> slicesByDepth(maxDepth + 1);
    for (const auto& slice : waitForCompressSlices) {
        if (slice.depth <= maxDepth) {
            slicesByDepth[slice.depth].push_back(slice);
        }
    }
    // 3. 分深度进行合并
    for (int depth = 0; depth <= maxDepth; ++depth) {
        if (slicesByDepth[depth].empty()) continue;
        DoCompressByDepth(slicesByDepth[depth], filterOutSmallFunc);
    }
}

void LeaksMemoryPythonTrace::DoCompressByDepth(std::vector<PythonTraceSlice>& depthSlices, const bool filterOutSmallFunc)
{
    // 遍历当前深度的调用序列, 尝试合并
    for (size_t i = 0; i < depthSlices.size();) {
        size_t startIdx = i;
        int depth = depthSlices[i].depth;
        uint64_t minStart = depthSlices[i].startTimestamp;
        uint64_t maxEnd = depthSlices[i].endTimestamp;
        std::string mergedFuncName = StringUtil::StrJoin("Merged: ", depthSlices[i].func);
        // 尝试从i开始，尽可能多的合并后续调用
        while (i + 1 < depthSlices.size()) {
            uint64_t nextSliceStart = depthSlices[i+1].startTimestamp;
            uint64_t gap = nextSliceStart - maxEnd;
            // 检查时间间隔(前序已校验max>min, 此处无回绕风险)
            if (gap  < ((maxTimestamp - minTimestamp) / SMALL_RATIO)) {
                // 间隔很小 可以合并
                i++;
                mergedFuncName.append(StringUtil::StrJoin(DEFAULT_MERGED_SEP, depthSlices[i].func));
                maxEnd = std::max(maxEnd, depthSlices[i].endTimestamp);
                continue;
            }
            // 不满足间隔时间小，停止合并
            break;
        }
        // 从 startIdx 到 i (包含) 的调用可以合并为一个
        if (i > startIdx) {
            PythonTraceSlice merged(mergedFuncName, minStart, maxEnd, depth);
            slices.push_back(merged);
            i++; // 移动到下一组的开始
            continue;
        }
        // 无可合并内容, 根据策略决定是丢弃还是放回
        if (!filterOutSmallFunc) {
            slices.push_back(depthSlices[i]);
        }
        i++;
    }
}

// [PYTHON_TRACE]

// [EVENT]
EventGroup::EventGroup(std::vector<MemoryEvent>& sortedEvents)
{
    for (auto &event : sortedEvents) {
            AddEvent(event);
    }
}

void EventGroup::AddEvent(const MemoryEvent& event)
{
    if (event.event == LEAKS_DUMP_EVENT::MALLOC) {
        if (mallocEvent.has_value()) {
            Server::ServerLog::Warn("Duplicated malloc events in the same group.");
            return;
        }
        mallocEvent = std::make_optional(event);
        return;
    }
    if (event.event == LEAKS_DUMP_EVENT::FREE) {
        if (freeEvent.has_value()) {
            Server::ServerLog::Warn("Duplicated free events in the same group.");
            return;
        }
        freeEvent = std::make_optional(event);
        return;
    }
    if (event.event == LEAKS_DUMP_EVENT::ACCESS) {
        accessEvents.push_back(event);
    }
}
// [EVENT]

// [BLOCK]
std::string MemoryBlockAttrs::ToJsonString()
{
    document_t blockAttrJson(kObjectType);
    auto& allocator = blockAttrJson.GetAllocator();
    JsonUtil::AddMember(blockAttrJson, BLOCK_EVENT_ATTR_SIZE_FIELD, size, allocator);
    JsonUtil::AddMember(blockAttrJson, BLOCK_EVENT_ATTR_GROUP_ID_FIELD, groupId, allocator);
    JsonUtil::AddMember(blockAttrJson, BLOCK_ATTR_FIRST_ACCESS_FILED, firstAccessTimestamp, allocator);
    JsonUtil::AddMember(blockAttrJson, BLOCK_ATTR_LAST_ACCESS_FILED, lastAccessTimestamp, allocator);
    for (auto &attrPair : extendAttrs) {
        JsonUtil::AddMember(blockAttrJson, attrPair.first, attrPair.second, allocator);
    }
    return JsonUtil::JsonDump(blockAttrJson);
}
std::optional<MemoryBlockAttrs> MemoryBlockAttrs::FromJson(std::string jsonString)
{
    MemoryBlockAttrs attrs;
    std::string error;
    auto attrsJsonDoc = JsonUtil::TryParse(jsonString, error);
    if (!error.empty() || !attrsJsonDoc.has_value()) {
        Server::ServerLog::Warn("Failed to parse block attrs: ", error);
        return std::nullopt;
    }
    auto &attrsJson = attrsJsonDoc.value();
    JsonUtil::SetByJsonKeyValue(attrs.size, attrsJson, "size");
    JsonUtil::SetByJsonKeyValue(attrs.groupId, attrsJson, "allocation_id");
    JsonUtil::SetByJsonKeyValue(attrs.firstAccessTimestamp, attrsJson, "first_access_timestamp");
    JsonUtil::SetByJsonKeyValue(attrs.lastAccessTimestamp, attrsJson, "last_access_timestamp");
    return attrs;
}
// [BLOCK]
} // Dic::Module::MemoryDetail