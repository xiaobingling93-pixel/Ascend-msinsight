/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERBIN_H
#define PROFILER_SERVER_PARSERBIN_H

#include "ParserFactory.h"
#include "TimelineRequestHandler.h"
#include "FileParser.h"
#include "TraceFileParser.h"

namespace Dic {
namespace Module {

class ParserBin : public ParserAlloc {
public:
    ParserBin();
    virtual ~ParserBin();

    void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request) final;
    void ParserBaseline(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
                        Global::BaselineInfo &baselineInfo) final;
    ProjectTypeEnum GetProjectType(const std::vector<std::string> &dataPath) final;

private:
    void HandleCompute(ImportActionResponse &response, const std::string &selectedFolder);
    std::vector<std::pair<std::string, std::string>> GetSimulationTraceFiles(const std::string &selectFilePath,
        ImportActionResBody &body);
    static void SetParseCallBack(FileParser &fileParser);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSERBIN_H
