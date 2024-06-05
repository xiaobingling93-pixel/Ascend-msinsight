/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERJSON_H
#define PROFILER_SERVER_PARSERJSON_H

#include "ParserFactory.h"
#include "TimelineRequestHandler.h"
#include "FileParser.h"

namespace Dic {
namespace Module {
class ParserJson : public ParserAlloc {
public:
    ParserJson();
    virtual ~ParserJson();

    void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request) final;
    ProjectTypeEnum GetProjectType(const std::vector<std::string> &dataPath) final;

private:
    std::vector<std::pair<std::string, std::string>> GetTraceFiles(const std::string &path, ImportActionResBody &body);
    std::vector<std::string> FindAllTraceFile(const std::string &path);
    std::vector<std::string> FindTraceFile(const std::string &path);
    void FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles);
    bool IsJsonValid(const std::string &fileName);
    static void ClusterProcess(const std::string &token, const std::string &selectedFolder,
                               std::map<std::string, std::vector<std::string>> &dataPathToDbMap,
                               const std::string &projectName);
    static void ClusterProcessAsyncStep(const std::string &token, const std::string &selectedFolder);

    void SetParseCallBack(const std::string token, FileParser &fileParser);

    bool isSimulation(std::string filePath);

    static void SendAllParseSuccess(const std::string &token);
    void ReloadDbPath(const std::vector<Global::ProjectExplorerInfo>& projectInfos, const ImportActionRequest &request);
    std::map<std::string, std::vector<std::string>> GetRankListMap(
            ImportActionResponse &response, const std::vector<Global::ProjectExplorerInfo> &projectInfos);
    void ParserTraceData(const std::map<std::string, std::vector<std::string>>& rankListMap,
                         const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSERJSON_H
