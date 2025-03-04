/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "BinFileParseUtil.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "SourceFileParser.h"
#include "QueryCachelineRecordHandler.h"
 
namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;
 
bool QueryCachelineRecordHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    std::unique_ptr<CachelineRecordResponse> responsePtr = std::make_unique<CachelineRecordResponse>();
    CachelineRecordResponse &response = *responsePtr;
    SetBaseResponse(*requestPtr, response);
    response.body.cachelineRecords = QueryCachelineRecord(SourceFileParser::Instance().GetFilePath());
    SetResponseResult(response, true);
    // add response to response queue in session
    Server::WsSessionManager::Instance().GetSession()->OnResponse(std::move(responsePtr));
    return true;
}

std::string QueryCachelineRecordHandler::QueryCachelineRecord(const std::string& filePath)
{
    std::ifstream file = OpenReadFileSafely(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Can't open file, please check file exist or not, file name: ", filePath);
        return {};
    }
    std::vector<Module::Source::Position> cacheLinePos =
        Source::SourceFileParser::Instance().GetPositionByType(Module::Source::DataTypeEnum::DISPLAY_CACHE);
    if (cacheLinePos.empty()) {
        return {};
    }
    Module::Source::Position &pair = cacheLinePos.at(0);
    return BinFileParseUtil::GetContentStr(file, pair);
}
 
} // Source
} // Module
} // Dic
