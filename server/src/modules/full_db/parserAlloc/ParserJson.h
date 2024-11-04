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
    void ParserBaseline(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
        Global::BaselineInfo &baselineInfo) final;
    ProjectTypeEnum GetProjectType(const std::vector<std::string> &dataPath) final;
    std::vector<std::string> GetParseFileByImportFile(const std::string &importFile, ProjectTypeEnum projectTypeEnum,
        std::string &error) final;
protected:
    bool CheckParseFileInfoSize(const Global::ParseFileInfo &parseFileInfo, std::vector<std::string> &jsonFiles) const;

private:
    std::vector<std::string> FindAllTraceFile(const std::string &path, std::string &error);
    std::vector<std::string> FindTraceFile(const std::string &path, std::string &error);
    void FindTraceFiles(const std::string &path, int depth, std::string &error, std::vector<std::string> &traceFiles);
    void FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles);
    bool IsJsonValid(const std::string &fileName);
    static void ClusterProcess(const std::string &selectedFolder, ProjectTypeEnum projectType,
        std::map<std::string, std::vector<std::string>> &dataPathToDbMap, const std::string &projectName);
    static void ClusterProcessAsyncStep(const std::string &selectedFolder);

    void SetParseCallBack(FileParser &fileParser);

    bool isSimulation(std::string filePath);

    void ReloadDbPath(const std::vector<Global::ProjectExplorerInfo> &projectInfos, const ImportActionRequest &request);
    std::map<std::string, std::vector<std::string>> GetRankListMap(
        const std::vector<Global::ProjectExplorerInfo> &projectInfos,
        std::map<std::string, std::vector<std::string>> &rankToFoldersMap);
    std::vector<std::string> GetJsonFileUnderFolder(const std::string &path);
    void ParserTraceData(const std::map<std::string, std::vector<std::string>> &rankListMap,
        const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request);
    static void FillBaseResponseInfo(const ImportActionRequest &request, ImportActionResponse &response,
                              const std::vector<Global::ProjectExplorerInfo> &projectInfos);
    static void ComputeSubirectoryList(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
        std::vector<std::string> &subdirectoryList);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSERJSON_H
