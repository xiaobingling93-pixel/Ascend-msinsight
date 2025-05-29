/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERBIN_H
#define PROFILER_SERVER_PARSERBIN_H

#include "ProjectParserFactory.h"
#include "TimelineRequestHandler.h"
#include "FileParser.h"
#include "TraceFileParser.h"


namespace Dic::Module {
using namespace Dic::Module::Global;
class ProjectParserBin : public ProjectParserBase {
public:
    ProjectParserBin() = default;
    ~ProjectParserBin() override = default;

    void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request) final;
    void ParserBaseline(const Global::ProjectExplorerInfo &projectInfo,
                        Global::BaselineInfo &baselineInfo) final;
    ProjectTypeEnum GetProjectType(const std::string &dataPath) final;

    std::vector<std::string> GetParseFileByImportFile(const std::string &importFile, std::string &error) override
    {
        return {importFile};
    }
    static void BuildProjectExploreInfo(ProjectExplorerInfo &projectInfo, const std::vector<std::string> &parsedFiles);
    static void BuildProjectInfoFromParseFile(ProjectExplorerInfo &projectInfo, const std::string& parsedFile);
    static std::string GetFileIdWithDb(const std::string& filePath);
private:
    void HandleCompute(ImportActionResponse &response, const std::string &selectedFolder);
    std::vector<std::pair<std::string, std::string>> GetSimulationTraceFiles(const std::string &selectFilePath,
        ImportActionResBody &body);
    static void SetParseCallBack(FileParser &fileParser);
};
} // end of namespace Dic::Module


#endif // PROFILER_SERVER_PARSERBIN_H
