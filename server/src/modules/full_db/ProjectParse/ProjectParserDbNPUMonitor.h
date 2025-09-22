/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_PARSERDBNPUMONITOR_H
#define PROFILER_SERVER_PARSERDBNPUMONITOR_H

#include "ProjectParserDb.h"

namespace Dic {
namespace Module {
class ProjectParserDbNPUMonitor : public ProjectParserDb {
public:
    ProjectParserDbNPUMonitor() = default;
    ~ProjectParserDbNPUMonitor() override = default;

    ProjectTypeEnum GetProjectType(const std::string &dataPath) final;
    std::vector<std::string> GetParseFileByImportFile(const std::string &importFile, std::string &error) final;
    static void BuildProjectExploreInfo(ProjectExplorerInfo& info, const std::vector<std::string>& parsedFiles);
    void ParserBaseline(const Global::ProjectExplorerInfo &projectInfo, Global::BaselineInfo &baselineInfo) final;
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_PARSERDBNPUMONITOR_H
