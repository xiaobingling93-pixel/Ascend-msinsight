//
// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_PARSERIPYNB_H
#define PROFILER_SERVER_PARSERIPYNB_H

#include "ProjectParserFactory.h"
#include "SystemMemoryDatabaseDef.h"

namespace Dic {
namespace Module {
class ProjectParserIpynb : public ProjectParserBase {
public:
    ProjectParserIpynb() = default;
    ~ProjectParserIpynb() override = default;

    void Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request) final;
    ProjectTypeEnum GetProjectType(const std::vector<std::string> &dataPath) final;
    std::vector<std::string> GetParseFileByImportFile(const std::string &importFile, std::string &error) override
    {
        return {importFile};
    }
    static void BuildProjectExploreInfo(ProjectExplorerInfo& projectInfo, const std::vector<std::string>& parsedFiles);
private:
    static void IpynbImportResponse(ImportActionRequest &request, const ProjectExplorerInfo &projectInfo,
                                    bool isDisplay);
    static void JupyterProcess(const std::string &file);
    static void SendJupyterInfo(std::string url);
};
}
}
#endif // PROFILER_SERVER_PARSERIPYNB_H
