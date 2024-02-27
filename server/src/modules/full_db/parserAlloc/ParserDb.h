/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERDB_H
#define PROFILER_SERVER_PARSERDB_H

#include "ParserFactory.h"
#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {

class ParserDb : public ParserAlloc {
public:
    ParserDb();
    virtual ~ParserDb();

    void Parser(const std::string &path, ImportActionRequest &request) final;
private:
    std::vector<std::string> GetReportFiles(const std::string &path, ImportActionResBody &body);
    void SetParseCallBack(std::string token);
    void SetBaseActionOfResponse(ImportActionResponse &response, std::string rankId,
                                 std::map<std::string, std::string> devicePaths);
    static void ClusterProcess(const std::string &token, const std::string &selectedFolder, bool isCluster);
    static void ClusterProcessAsyncStep(const std::string &token, const std::string &selectedFolder);
};

} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSERDB_H
