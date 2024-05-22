/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERFACTORY_H
#define PROFILER_SERVER_PARSERFACTORY_H


#include <iostream>
#include <memory>
#include "TimelineRequestHandler.h"
#include "GlobalDefs.h"

namespace Dic {
namespace Module {

class ParserAlloc {
public:
    ParserAlloc() = default;
    virtual ~ParserAlloc() = default;

    virtual void Parser(const std::string &path, ImportActionRequest &request){};
    static void ParseEndCallBack(const std::string &token, const std::string &fileId, bool result,
                                 const std::string &message);
protected:
    static bool curIsIpynb;
    std::string curScene;

    static void ParseClusterEndProcess(const std::string token, std::string result);
    static void SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UnitTrack>> &metaData);
    static std::string GetFileId(const std::string &filePath);
    static std::string GetDbPath(const std::string &filePath, const int index);
    static bool CheckIsCluster(const std::string &filePath);
    static void SendParseSuccessEvent(const std::string &token, const std::string &fileId);
    static void SendParseFailEvent(const std::string &token, const std::string &fileId, const std::string &message);

    void SetBaseActionOfResponse(ImportActionResponse &response,
        std::pair<std::string, std::vector<std::string>> rankEntry);
    bool CheckIfClusterAndReset(const std::string &path, int filesSize, ImportActionResBody &body, bool isDb);
};

class ParserFactory {
public:
    static std::shared_ptr<ParserAlloc> ParserImport(ParserType allocType);
};
} // end of namespace Module
} // end of namespace Dic


#endif // PROFILER_SERVER_PARSERFACTORY_H
