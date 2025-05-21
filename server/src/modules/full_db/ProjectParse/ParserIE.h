/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERIE_H
#define PROFILER_SERVER_PARSERIE_H
#include "ServitizationOpenApi.h"
#include "ProjectParserFactory.h"
#include "FileParser.h"
namespace Dic::Module {
class ParserIE : public ProjectParserBase {
public:
    ParserIE() = default;
    ~ParserIE() override;

    void Parser(const std::vector<Global::ProjectExplorerInfo>& projectInfos, ImportActionRequest& request) final;
    ProjectTypeEnum GetProjectType(const std::string& dataPath) final;
    std::vector<std::string> GetParseFileByImportFile(const std::string& importFile, std::string& error);
    static void BuildProjectExploreInfo(ProjectExplorerInfo& projectInfo, const std::vector<std::string>& parsedFiles);
    bool ExistIEFile(const std::string& file);

protected:
    std::shared_ptr<IE::ServitizationOpenApi> servitizationOpenApi = std::make_shared<IE::ServitizationOpenApi>();

private:
    std::vector<std::string> FindIEFile(const std::string& path);
    void SetParseCallBack(FileParser& fileParser);
    std::unordered_map<std::string, std::string> GetRankListMap(
        const std::vector<Global::ProjectExplorerInfo>& projectInfos);
    void ParserTraceData(const std::unordered_map<std::string, std::string>& rankListMap);
    static void FillBaseResponseInfo(const ImportActionRequest& request, ImportActionResponse& response,
                                     const std::vector<ProjectExplorerInfo>& projectInfos);
};
}  // namespace Dic::Module

#endif  // PROFILER_SERVER_PARSERIE_H
