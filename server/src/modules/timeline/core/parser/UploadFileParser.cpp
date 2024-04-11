/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "UploadFileParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic;
using namespace Dic::Server;

UploadFileParser &UploadFileParser::Instance()
{
    static UploadFileParser instance;
    return instance;
}
UploadFileParser::UploadFileParser() {}

UploadFileParser::~UploadFileParser()
{
    threadPool.ShutDown();
}

bool UploadFileParser::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
    const std::string &selectedFile) {}

void UploadFileParser::Parse(UploadFileRequest request)
{
    threadPool.AddTask([this, request]() { return this->ParseTask(request); });
}

bool UploadFileParser::ResetByFiles(const std::vector<std::string> &filePaths)
{
    for (const auto &filePath : filePaths) {
        std::string fileId = FileUtil::PathPreprocess(filePath);
        if (singleFileDataMap.count(fileId) == 0) {
            return true;
        }
        SingleFileData &singleFileData = singleFileDataMap.at(fileId);
        singleFileData.reset.store(true);
    }
    return true;
}

bool UploadFileParser::ResetAllFiles()
{
    for (auto &entry : singleFileDataMap) {
        SingleFileData &singleFileData = entry.second;
        singleFileData.reset.store(true);
    }
    return true;
}

void UploadFileParser::ParseTask(UploadFileRequest request)
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
        ParseLast(fileId, request);
    }
}

void UploadFileParser::ParseLast(std::string fileId, UploadFileRequest request)
{
    SingleFileData &singleFileData = singleFileDataMap.at(fileId);
    // 后置处理时需要等待前置分片处理完成
    int attempts = 0;
    while (singleFileData.currentSize != request.params.slice.count) {
        if (attempts >= MAX_ATTEMPT) {
            ServerLog::Error("Error in accepting file slice data, file is ", fileId, ",accept size is ",
                static_cast<int>(singleFileData.currentSize), ",count is", request.params.slice.count);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
        ++attempts;
    }

    std::unique_lock<std::shared_mutex> lock(singleFileData.readWriteMutex);

    std::stringstream strStream;
    for (auto &str : singleFileData.stringArray) {
        strStream << str;
    }
    std::string result = strStream.str();
    EventParser("", fileId).Parse(0, result);

    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to open traceDatabase for ParseLast. rankId:", fileId);
        return;
    }
    auto database = std::dynamic_pointer_cast<JsonTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to convert VirtualTraceDatabase to JsonTraceDatabase in ParseLast.");
        return;
    }
    database->CreateIndex();
    database->UpdateDepth();

    // 根据是否重置发送解析完成消息
    bool reset = singleFileData.reset.load();
    ParseEndSendResp(fileId, request, !reset);
    ParserAlloc::ParseEndCallBack(request.token, fileId, true, "");
}

void UploadFileParser::InitDataBase(std::string fileId)
{
    std::string tmpDir = SystemUtil::GetTempDir();
    std::string dbName = StringUtil::GetHashStrName(fileId);
    std::string dbPath = tmpDir + "/" + dbName + ".db";

    DataBaseManager::Instance().CreatConnectionPool(fileId, dbPath);
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection,fileId:", fileId);
        return;
    }
    auto database = std::dynamic_pointer_cast<JsonTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr || !(database->DropAllTable() && database->CreateTable())) {
        ServerLog::Error("Failed to open traceDatabase. fileId:", fileId);
        ParseEndCallBack(fileId, false, "Failed to open db file. Please try again.");
        return;
    }
}

void UploadFileParser::ParseEndSendResp(const std::string &fileId, const UploadFileRequest &request,
    const bool result) const
{
    std::unique_ptr<UploadFileResponse> responsePtr = std::make_unique<UploadFileResponse>();
    UploadFileResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);
    response.body.isCluster = false;
    response.body.reset = false;
    ModuleRequestHandler::SetResponseResult(response, true);

    Action action;
    action.cardName = fileId;
    action.rankId = fileId;
    action.cardPath = fileId;
    action.result = result;
    response.body.result.emplace_back(action);

    WsSession &session = *WsSessionManager::Instance().GetSession(request.token);
    session.OnResponse(std::move(responsePtr));
}


std::tuple<std::string, std::string, std::string> UploadFileParser::SplitValidJsonStr(const std::string &fileContent)
{
    const std::pair<int64_t, int64_t> position = GetSplitPosition(fileContent);

    std::string prefixStr = fileContent.substr(0, position.first);
    std::string middleStr = fileContent.substr(position.first, position.second - position.first);
    std::string suffixStr = fileContent.substr(position.second);

    return std::tuple<std::string, std::string, std::string>(prefixStr, middleStr, suffixStr);
}

std::pair<int64_t, int64_t> UploadFileParser::GetSplitPosition(const std::string &fileContent)
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

void UploadFileParser::ParseEndCallBack(const std::string &fileId, bool result, const std::string &message)
{
    if (!(result && Timeline::ParserStatusManager::Instance().SetFinishStatus(fileId))) {
        result = false;
    }
    auto &instance = UploadFileParser::Instance();
    if (instance.paserEndCallback != nullptr) {
        instance.paserEndCallback(fileId, result, message);
    }
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic