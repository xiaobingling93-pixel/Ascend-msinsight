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

#ifndef PROFILER_SERVER_TRITONSERVICE_H
#define PROFILER_SERVER_TRITONSERVICE_H
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "GlobalDefs.h"

namespace Dic::Module::Triton {

struct TritonTensorBlock;
/*
 * 对应一次tensor alloc
 */
struct TritonTensorSegment {
    uint64_t size{0};
    bool tmpBuf{false};
    std::string buffer;
    std::string sourceLocation;
    uint64_t start{0};
    uint64_t end{0};
    uint64_t allocate{0};
    std::vector<TritonTensorBlock> blocks;
};

struct TritonTensorSegment;
/*
 * 开启double buffer时会一个tensor会对应多个block
 */
struct TritonTensorBlock {
public:
    TritonTensorBlock() = default;
    TritonTensorBlock(const TritonTensorSegment& segment)
    {
        id = segment.allocate;
        allocate = segment.allocate;
        start = segment.start;
        end = segment.end;
        tmpBuf = segment.tmpBuf;
    }
    std::string GetAttrStr() const
    {
        document_t attr(rapidjson::kObjectType);
        auto& allocator = attr.GetAllocator();
        JsonUtil::AddMember(attr, "buffer", buffer, allocator);
        JsonUtil::AddMember(attr, "offset", offset, allocator);
        JsonUtil::AddMember(attr, "sourceLocation", sourceLocation, allocator);
        JsonUtil::AddMember(attr, "allocLine", allocate, allocator);
        JsonUtil::AddMember(attr, "tmpBuf", tmpBuf, allocator);
        return JsonUtil::JsonDump(attr);
    }
    uint64_t id{};
    uint64_t offset{0};
    uint64_t size{0};
    uint64_t allocate{0};
    uint64_t start{0};
    uint64_t end{0};
    bool tmpBuf{false};
    std::string sourceLocation; // 默认为空，当前端返回纯block形式时才赋值
    std::string buffer;         // 默认为空，当前端返回纯block形式时才赋值
};

struct TritonMemeHeader {
    std::string kernelName;
};

class TritonService {
public:
    static TritonService& Instance();
    TritonService(const TritonService&) = delete;
    TritonService(TritonService&&) = delete;
    TritonService& operator=(const TritonService&) = delete;
    void Reset();
    void UpdateRecord(std::vector<TritonTensorSegment>&& segment);
    void SetHeader(TritonMemeHeader&& header);
    [[nodiscard]] TritonMemeHeader GetHeader() const { return header_; }
    [[nodiscard]] std::vector<TritonTensorSegment> QuerySegmentsContainRange(uint64_t timestamp) const;
    [[nodiscard]] std::vector<TritonTensorBlock> QueryBlocksContainRange(uint64_t start, uint64_t end) const;
private:
    TritonService() = default;
    TritonMemeHeader header_;
    std::vector<TritonTensorSegment> segments_;  // 按start end 进行升序排序
};
}

#endif //PROFILER_SERVER_TRITONSERVICE_H
