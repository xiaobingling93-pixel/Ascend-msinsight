/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERDB_H
#define PROFILER_SERVER_PARSERDB_H

#include "ParserFactory.h"
#include "TimelineRequestHandler.h"
#include "WsSessionManager.h"

namespace Dic {
namespace Module {

using HostInfo = std::map<std::string, std::vector<std::string>>;

class ParserDb : public ParserAlloc {
public:
    ParserDb();
    virtual ~ParserDb();

    void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request) final;
    ProjectTypeEnum GetProjectType(const std::vector<std::string> &dataPath) final;
private:
    std::map<std::string, HostInfo> GetReportFiles(const std::string &path, ImportActionResBody &body);
    void SetParseCallBack(std::string token);
    static void SetBaseActionOfResponse(ImportActionResponse &response, const std::string& rankId,
                                        const std::string& host, const std::string& dbFile);
    static void ClusterProcess(const std::string &token, const std::string &selectedFolder, bool isCluster);
    static void ClusterProcessAsyncStep(const std::string &token, const std::string &selectedFolder);
};

} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSERDB_H
