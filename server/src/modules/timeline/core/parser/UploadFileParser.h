/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_UPLOADFILEPARSER_H
#define PROFILER_SERVER_UPLOADFILEPARSER_H

#include <map>
#include <array>
#include <string>
#include <shared_mutex>
#include "document.h"
#include "rapidjson.h"
#include "document.h"
#include "FileParser.h"
#include "ThreadPool.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TimelineRequestHandler.h"
#include "ParserStatusManager.h"
#include "EventParser.h"
#include "FileUtil.h"
#include "ThreadPool.h"
#include "SystemUtil.h"
#include "ParserFactory.h"
#include "ModuleRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
const int MAX_SLICE_SIZE = 1024 * 10;
const int MAX_STR_SIZE = 1024 * 1024 * 50;
const int MAX_ATTEMPT = 50;
const int WAIT_TIME = 100;

struct SingleFileData {
    std::array<std::string, MAX_SLICE_SIZE> stringArray;
    std::atomic_int currentSize;
    std::atomic_bool reset;
    std::shared_mutex readWriteMutex;

    explicit SingleFileData() : currentSize(0), reset(false) {}
};

class UploadFileParser : public FileParser {
public:
    static UploadFileParser &Instance();

    UploadFileParser();
    ~UploadFileParser() override;

    bool Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
        const std::string &selectedFile) override;
    void Parse(UploadFileRequest request);
    bool ResetByFiles(const std::vector<std::string> &filePaths);
    bool ResetAllFiles();

private:
    const int maxThreadNum = 4;
    std::unique_ptr<ThreadPool> threadPool;
    std::map<std::string, SingleFileData> singleFileDataMap;

    std::string InitDataBase(std::string fileId);
    static bool CheckParseTask(UploadFileRequest request, int sliceIndex, SingleFileData &singleFileData,
                               std::string fileId);
    void ParseTask(UploadFileRequest request);
    void ParseSliceData(const UploadFileRequest& request, const std::string& fileId);
    void ParseLast(std::string fileId, UploadFileRequest request);
    void ParseEndSendResp(const std::string &fileId, const UploadFileRequest &request, const bool result) const;
    std::tuple<std::string, std::string, std::string> SplitValidJsonStr(const std::string &fileContent);
    static std::pair<int64_t, int64_t> GetSplitPosition(const std::string &fileContent);
    static void ParseEndCallBack(const std::string &fileId, bool result, const std::string &message);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_UPLOADFILEPARSER_H
