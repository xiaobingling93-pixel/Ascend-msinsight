/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERDB_H
#define PROFILER_SERVER_PARSERDB_H

#include "ProjectParserFactory.h"
#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
using HostInfo = std::map<std::string, std::vector<std::string>>;

class ProjectParserDb : public ProjectParserBase {
public:
    ProjectParserDb() = default;
    ~ProjectParserDb() override = default;

    void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request) final;
    void ParserBaseline(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
        Global::BaselineInfo &baselineInfo) final;
    ProjectTypeEnum GetProjectType(const std::vector<std::string> &dataPath) final;
    std::vector<std::string> GetParseFileByImportFile(const std::string &importFile, std::string &error) final;
    static void BuildProjectExploreInfo(ProjectExplorerInfo& info, const std::vector<std::string>& parsedFiles);
private:
    std::map<std::string, HostInfo> GetReportFiles(const std::vector<std::string> &reportFiles);
    void SetParseCallBack();
    static void SetBaseActionOfResponse(ImportActionResponse &response, const std::string &rankId,
        const std::string &host, const std::string &dbFile);
    static void SetHostInfo(std::map<std::string, HostInfo> &hostInfoMap, ImportActionResponse &response);
    static void ClusterProcess(const std::string &selectedFolder, bool isCluster, ProjectTypeEnum curProjectTypeEnum,
        std::map<std::string, std::vector<std::string>> &dataPathToDbMap, const std::string &projectName);
    static void ParseClusterBaselineInfo(const std::vector<Global::ProjectExplorerInfo> &projectInfos);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSERDB_H
