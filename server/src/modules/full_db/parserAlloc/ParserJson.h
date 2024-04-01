/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERJSON_H
#define PROFILER_SERVER_PARSERJSON_H

#include "ParserFactory.h"
#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
class ParserJson : public ParserAlloc {
public:
    ParserJson();
    virtual ~ParserJson();

    void Parser(const std::string &path, ImportActionRequest &request) final;

private:
    std::vector<std::pair<std::string, std::string>> GetTraceFiles(const std::string &path, ImportActionResBody &body);
    std::vector<std::string> FindAllTraceFile(const std::string &path);
    std::vector<std::string> FindTraceFile(const std::string &path);
    void FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles);
    bool IsJsonValid(const std::string &fileName);
    static void ClusterProcess(const std::string &token, const std::string &selectedFolder);
    static void ClusterProcessAsyncStep(const std::string &token, const std::string &selectedFolder);

    void SetParseCallBack(std::string token);

    static void SendAllParseSuccess(const std::string &token);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSERJSON_H
