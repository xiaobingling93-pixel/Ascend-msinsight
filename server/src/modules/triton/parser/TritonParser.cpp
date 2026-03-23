// /*
//  * -------------------------------------------------------------------------
//  * This file is part of the MindStudio project.
//  * Copyright (c)  $YEAR$ Huawei Technologies Co.,Ltd.
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

#include "TritonParser.h"
#include <algorithm>
#include <array>
#include "FileUtil.h"
#include "JsonUtil.h"
#include "ParserStatusManager.h"
#include "ServerLog.h"
#include "ThreadPool.h"

#include "ProtocolDefs.h"
#include "TimelineProtocolEvent.h"
#include "TritonProtocolEvent.h"
#include "TritonService.h"
#include "WsSender.h"

namespace Dic::Module::Triton {
using namespace Dic::Server;

namespace {
template<size_t N>
bool HasExpectedMembers(const json_t &jsonObj, const std::array<const char *, N> &expectedKeys)
{
    if (!jsonObj.IsObject()) {
        return false;
    }
    return std::all_of(
        expectedKeys.begin(), expectedKeys.end(), [&jsonObj](const char *key) { return jsonObj.HasMember(key); });
}
} // namespace

TritonParser &TritonParser::Instance()
{
    static TritonParser instance;
    return instance;
}
void TritonParser::Parse(const std::string &parseDir)
{
    Timeline::ParserStatusManager::Instance().WaitStartParse();
    BeforeParse(parseDir);
    auto future = ThreadPool::Instance().AddTask([this, parseDir]() { return ParseImpl(parseDir); },
                                                 TraceIdManager::GetTraceId());
    auto result = future.get();
    AfterParse(result);
}
bool TritonParser::IsParsed(const std::string &filePath) const
{
    if (filePath.empty()) {
        return false;
    }
    if (!FileUtil::CheckPathSecurity(filePath, CHECK_FILE_READ)) {
        return false;
    }
    std::string fileName = FileUtil::GetFileName(filePath);
    return fileName == tritonMemFileName;
}

void TritonParser::BeforeParse(const std::string &parsedDir)
{
    if (!FileUtil::CheckPathSecurity(parsedDir, CHECK_FILE_READ)) {
        ServerLog::Error("Triton file dir is not safe, please check log for more information");
        return;
    }
    parsedFiles.clear();
    parsedFiles.push_back(parsedDir);
}

void TritonParser::AfterParse(const ParseResult &result) const
{
    if (!result) {
        auto event = std::make_unique<Protocol::ParseFailEvent>();
        event->moduleName = Protocol::MODULE_TIMELINE;
        event->result = false;
        std::string path = parsedFiles.empty() ? "" : parsedFiles.front();
        event->body.rankId = path;
        event->body.error = result.GetErrorMsg();
        event->body.dbPath = path;
        Dic::SendEvent(std::move(event));
        return;
    }
    auto event = std::make_unique<Protocol::TritonParseSuccessEvent>();
    event->moduleName = Protocol::MODULE_TRITON;
    event->result = true;
    Protocol::TritonParseSuccessEventBody body;
    event->body = body;
    Dic::SendEvent(std::move(event));
}

ParseResult TritonParser::ParseImpl(const std::string &parsedDir)
{
    if (parsedFiles.empty()) {
        ServerLog::Error("Not found need parsed File.");

        return {false, "Not found triton file"};
    }
    std::for_each(parsedFiles.begin(), parsedFiles.end(), [this](const std::string &filePath) {
        auto result = ParseOneTriton(filePath);
    });
    return {true, "success"};
}

bool TritonParser::CheckFileValid(const std::string &fileName, std::string &error)
{
    if (fileName.empty()) {
        error = "Triton file name is required";
        return false;
    }
    if (!FileUtil::CheckPathSecurity(fileName, CHECK_FILE_READ)) {
        error = "Triton file not satisfy safety requirement, please check the log for mor information";
        return false;
    }
    return true;
}
ParseResult TritonParser::ParseOneTriton(const std::string &memFile)
{
    document_t jsonDoc = JsonUtil::ReadJsonFromFile(memFile);
    if (!CheckDataValid(jsonDoc)) {
        return {false, "Invalid Data"};
    }
    TritonMemeHeader header;
    auto &headerJson = jsonDoc["Header"];
    JsonUtil::SetByJsonKeyValue(header.kernelName, headerJson, "KernelName");
    std::vector<TritonTensorSegment> tensorSegments;
    TritonService::Instance().SetHeader(std::move(header));
    auto &jsonRecord = jsonDoc["Record"];
    tensorSegments.reserve(jsonRecord.Size());
    for (const json_t &recordItem : jsonRecord.GetArray()) {
        TritonTensorSegment segment;
        JsonUtil::SetByJsonKeyValue(segment.allocate, recordItem, "alloc_time_in_ir");
        JsonUtil::SetByJsonKeyValue(segment.buffer, recordItem, "buffer");
        JsonUtil::SetByJsonKeyValue(segment.sourceLocation, recordItem, "source_location");
        JsonUtil::SetByJsonKeyValue(segment.tmpBuf, recordItem, "is_tmpbuf");
        auto lifeTime = JsonUtil::GetVector<uint64_t>(recordItem, "life_time_in_ir");
        uint64_t start = lifeTime[0];
        uint64_t end = lifeTime[1];
        segment.start = start;
        segment.end = end;
        uint64_t extend = recordItem["extent"].GetUint64();
        uint64_t blockCount = recordItem["offset"].Size();
        segment.size = extend * blockCount;
        segment.blocks.reserve(blockCount);
        for (const auto &block : recordItem["offset"].GetArray()) {
            TritonTensorBlock blockData(segment);
            blockData.offset = block.GetUint64();
            blockData.size = extend;
            segment.blocks.emplace_back(std::move(blockData));
        }
        tensorSegments.emplace_back(std::move(segment));
    }
    TritonService::Instance().UpdateRecord(std::move(tensorSegments));
    return {true, "Success"};
}

bool TritonParser::CheckDataValid(document_t &json)
{
    if (json.IsNull()) {
        return false;
    }
    if (!json.IsObject()) {
        return false;
    }

    constexpr std::array<const char *, 2> rootKeys = {"Header", "Record"};
    if (!HasExpectedMembers(json, rootKeys)) {
        ServerLog::Error("Triton json root required keys are missing");
        return false;
    }

    const json_t &header = json["Header"];
    constexpr std::array<const char *, 1> headerKeys = {"KernelName"};
    if (!HasExpectedMembers(header, headerKeys)) {
        ServerLog::Error("Triton Header required keys are missing");
        return false;
    }
    if (!header["KernelName"].IsString()) {
        ServerLog::Error("Triton Header value types are invalid: KernelName must be string");
        return false;
    }

    if (!json["Record"].IsArray()) {
        ServerLog::Error("Triton Record must be array");
        return false;
    }

    constexpr std::array<const char *, 7> recordKeys = {
        "alloc_time_in_ir", "buffer", "extent", "is_tmpbuf", "life_time_in_ir", "offset", "source_location"};
    for (const json_t &recordItem : json["Record"].GetArray()) {
        if (!HasExpectedMembers(recordItem, recordKeys)) {
            ServerLog::Error("Triton Record item required keys are missing");
            return false;
        }
        bool typesOk = recordItem["alloc_time_in_ir"].IsInt64() && recordItem["buffer"].IsString() &&
            recordItem["extent"].IsInt64() && recordItem["is_tmpbuf"].IsBool() && recordItem["offset"].IsArray() &&
            recordItem["source_location"].IsString();
        if (!typesOk) {
            ServerLog::Error("Triton Record item value types are invalid");
            return false;
        }

        const json_t &lifeTime = recordItem["life_time_in_ir"];
        if (!lifeTime.IsArray()) {
            ServerLog::Error("Triton life_time_in_ir format is invalid");
            return false;
        }
    }

    return true;
}
} // namespace Dic::Module::Triton
// Module
// Dic
