/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "UploadFileHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Module::Timeline;
using namespace Dic::Server;
void UploadFileHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    UploadFileRequest &request = dynamic_cast<UploadFileRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    ServerLog::Info("File upload start, token = ", StringUtil::AnonymousString(sessionToken));
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(sessionToken),
            ", command = ", command);
        return;
    }

    threadPool.AddTask([this, request]() { return this->ParseTask(request); });
}

void UploadFileHandler::ParseTask(UploadFileRequest request)
{
    std::string fileId = FileUtil::PathPreprocess(request.params.fileAttr.path);
    if (!DataBaseManager::Instance().HasFileId(DatabaseType::TRACE, fileId)) {
        InitDataBase(fileId);
    }
    if (singleFileDataMap.count(fileId) == 0) {
        singleFileDataMap[fileId];
    }
    SingleFileData &singleFileData = singleFileDataMap.at(fileId);

    int sliceIndex = request.params.slice.index;
    if (sliceIndex > singleFileData.stringArray.size()) {
        ServerLog::Error("Failed to parser upload file ", fileId, ",current index is ", sliceIndex, "max size is ",
            singleFileData.stringArray.size());
        return;
    }
    int textSize = request.params.text.size();
    if (textSize > MAX_STR_SIZE) {
        ServerLog::Error("Failed to parser upload file ", fileId, ",current content size is ", textSize, "max size is ",
            MAX_STR_SIZE);
        return;
    }

    // 处理分片json内容
    std::string content = request.params.text;
    const std::tuple<std::string, std::string, std::string> tuple = SplitValidJsonStr(content);
    const std::string middleJson = std::get<1>(tuple);
    size_t pos = middleJson.find('{');
    if (pos != std::string::npos) {
        std::string part = middleJson.substr(pos);
        std::string jsonArray = "[" + part + "}]";
        EventParser("", fileId).Parse(sliceIndex, jsonArray);
    }
    // 剩余字段存入array
    std::shared_lock<std::shared_mutex> lock(singleFileData.readWriteMutex);
    auto [firstElement, secondElement, thirdElement] = tuple;
    singleFileData.stringArray[sliceIndex] = firstElement + thirdElement;
    ++singleFileData.currentSize;
    lock.unlock();

    bool isLast = request.params.slice.isLast;
    if (isLast) {
        int attempts = 0;
        while (singleFileData.currentSize != request.params.slice.count) {
            if (attempts >= MAX_ATTEMPT) {
                ServerLog::Warn("Error in accepting file slice data, file is ", fileId, ",accept size is ",
                    static_cast<int>(singleFileData.currentSize), ",count is", request.params.slice.count);
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
            ++attempts;
        }
        ParseLast(fileId, request);
    }
}

void UploadFileHandler::ParseLast(std::string fileId, UploadFileRequest request)
{
    SingleFileData &singleFileData = singleFileDataMap.at(fileId);
    std::unique_lock<std::shared_mutex> lock(singleFileData.readWriteMutex);

    std::stringstream strStream;
    for (auto &str : singleFileData.stringArray) {
        strStream << str;
    }
    std::string result = strStream.str();
    EventParser("", fileId).Parse(0, result);

    auto database = std::dynamic_pointer_cast<JsonTraceDatabase, VirtualTraceDatabase>(
        DataBaseManager::Instance().GetTraceDatabase(fileId));
    database->CreateIndex();
    database->UpdateDepth();
    // send response and event
    ParseEndSendResp(fileId, request);
    ParserAlloc::ParseEndCallBack(request.token, fileId, true, "");
}

void UploadFileHandler::InitDataBase(std::string fileId)
{
    std::string tmpDir = SystemUtil::GetTempDir();
    std::string dbName = StringUtil::GetHashStrName(fileId);
    std::string dbPath = tmpDir + "/" + dbName + ".db";

    DataBaseManager::Instance().CreatConnectionPool(fileId, dbPath);
    auto database = std::dynamic_pointer_cast<JsonTraceDatabase, VirtualTraceDatabase>(
        DataBaseManager::Instance().GetTraceDatabase(fileId));
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection,fileId:", fileId);
        return;
    }
    if (!(database->DropAllTable() && database->CreateTable())) {
        ServerLog::Error("Failed to open traceDatabase. fileId:", fileId);
        return;
    }
}

void UploadFileHandler::ParseEndSendResp(const std::string &fileId, const UploadFileRequest &request) const
{
    std::unique_ptr<UploadFileResponse> responsePtr = std::make_unique<UploadFileResponse>();
    UploadFileResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    response.body.isCluster = false;
    response.body.reset = false;
    SetResponseResult(response, true);

    Action action;
    action.cardName = fileId;
    action.rankId = fileId;
    action.cardPath = fileId;
    action.result = true;
    response.body.result.emplace_back(action);

    WsSession &session = *WsSessionManager::Instance().GetSession(request.token);
    session.OnResponse(std::move(responsePtr));
}


std::tuple<std::string, std::string, std::string> UploadFileHandler::SplitValidJsonStr(const std::string &fileContent)
{
    const std::pair<int64_t, int64_t> position = GetSplitPosition(fileContent);

    std::string prefixStr = fileContent.substr(0, position.first);
    std::string middleStr = fileContent.substr(position.first, position.second - position.first);
    std::string suffixStr = fileContent.substr(position.second);

    return std::tuple<std::string, std::string, std::string>(prefixStr, middleStr, suffixStr);
}

std::pair<int64_t, int64_t> UploadFileHandler::GetSplitPosition(const std::string &fileContent)
{
    std::pair<int64_t, int64_t> res;
    std::regex regex(R"(\}\s*,\s*\{)"); // 匹配 "}, {"

    // 第一个匹配
    std::smatch match;
    if (std::regex_search(fileContent, match, regex)) {
        res.first = match.position();
    }

    // 最后一个匹配
    std::string::const_iterator it = fileContent.cbegin();
    std::string::const_iterator end = fileContent.cend();
    std::regex_iterator<std::string::const_iterator> iter(it, end, regex);
    std::regex_iterator<std::string::const_iterator> enditer;
    std::regex_iterator<std::string::const_iterator> lastMatchIter;
    while (iter != enditer) {
        lastMatchIter = iter;
        ++iter;
    }
    if (lastMatchIter != enditer) {
        std::smatch lastMatch = *lastMatchIter;
        res.second = lastMatch.position();
    }

    return res;
}
} // Global
} // Module
} // Dic