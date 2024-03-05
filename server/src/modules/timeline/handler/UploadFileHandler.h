/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_UPLOADFILEHANDLER_H
#define PROFILER_SERVER_UPLOADFILEHANDLER_H

#include <array>
#include <string>
#include <shared_mutex>
#include "ServerLog.h"
#include "FileSelector.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "EventParser.h"
#include "ParserFactory.h"
#include "FileUtil.h"
#include "TimelineRequestHandler.h"
#include "ThreadPool.h"
#include "SystemUtil.h"

namespace Dic {
namespace Module {
namespace Timeline {
const int MAX_SLICE_SIZE = 1024 * 10;
const int MAX_STR_SIZE = 1024 * 1024;
const int MAX_ATTEMPT = 5;
const int WAIT_TIME = 2;

struct SingleFileData {
    std::array<std::string, MAX_SLICE_SIZE> stringArray;
    std::atomic_int currentSize;
    std::shared_mutex readWriteMutex;

    explicit SingleFileData() : currentSize(0) {}
};

class UploadFileHandler : public TimelineRequestHandler {
public:
    UploadFileHandler()
    {
        command = REQ_RES_UPLOAD_FILE;
    }
    ~UploadFileHandler() override = default;
    void HandleRequest(std::unique_ptr<Request> requestPtr) override;

private:
    ThreadPool threadPool = ThreadPool(SystemUtil::GetCpuCoreCount());
    std::map<std::string, SingleFileData> singleFileDataMap;

    void InitDataBase(std::string fileId);
    void ParseTask(UploadFileRequest request);
    void ParseLast(std::string fileId, UploadFileRequest request);
    void ParseEndSendResp(const std::string &fileId, const UploadFileRequest &request) const;
    std::tuple<std::string, std::string, std::string> SplitValidJsonStr(const std::string &fileContent);
    static std::pair<int64_t, int64_t> GetSplitPosition(const std::string &fileContent);
};
} // Timeline
} // Module
} // Dic
#endif // PROFILER_SERVER_UPLOADFILEHANDLER_H
