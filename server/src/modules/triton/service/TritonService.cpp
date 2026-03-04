// /*
//  * -------------------------------------------------------------------------
//  * This file is part of the MindStudio project.
//  * Copyright (c)  2026 Huawei Technologies Co.,Ltd.
//  *
//  * MindStudio is licensed under Mulan PSL v2.
//  * You can use this software according to the terms and conditions of the Mulan PSL v2.
//  * You may obtain a copy of Mulan PSL v2 at:
//  *
//  *          http://license.coscl.org.cn/MulanPSL2
//  *
//  * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
//  * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
//  * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
//  * See the Mulan PSL v2 for more details.
//  * -------------------------------------------------------------------------
//  *


#include "TritonService.h"
#include <algorithm>



namespace Dic::Module::Triton {
TritonService& TritonService::Instance()
{
    static TritonService instance;
    return instance;
}

void TritonService::Reset()
{
    header_ = {};
    segments_.clear();
}

void TritonService::UpdateRecord(std::vector<TritonTensorSegment>&& segment)
{
    // 插入数据本身有序
    segments_ = std::move(segment);
}

void TritonService::SetHeader(TritonMemeHeader&& header)
{
    header_ = std::move(header);
}

std::vector<TritonTensorSegment> TritonService::QuerySegmentsContainRange(uint64_t timestamp) const
{
    std::vector<TritonTensorSegment> result;
    std::for_each(segments_.begin(), segments_.end(), [&result, timestamp](const TritonTensorSegment& seg)
    {
        if (seg.start <= timestamp && timestamp <= seg.end)
        {
            result.push_back(seg);
        }
    });
    return result;
}
std::vector<TritonTensorBlock> TritonService::QueryBlocksContainRange(uint64_t start, uint64_t end) const
{
    std::vector<TritonTensorBlock> result;
    auto tranSegmentToBlock = [](const TritonTensorSegment& item, std::vector<TritonTensorBlock>& blocks) {
        for (auto block : item.blocks) { // 复制语义循环，完成数据复制
            block.sourceLocation = item.sourceLocation;
            block.buffer = item.buffer;
            blocks.push_back(std::move(block));
        }
    };
    if (start == 0 && end == std::numeric_limits<uint64_t>::max()) {
        for (const auto& segment : segments_) {
            tranSegmentToBlock(segment, result);
        }
    }
    if (start > end) {
        return result;
    }
    for (const auto& segment : segments_) {
        if (segment.start > start || segment.end < end) {
            break;
        }
        if (segment.start <= start && segment.end >= end) {
            tranSegmentToBlock(segment, result);
        }
    }
    return result;
}
} // Triton
// Module
// Dic
