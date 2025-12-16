/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <algorithm>
#include "ProjectExplorerManager.h"
#include "ProjectParserFactory.h"
#include "BaselineManager.h"
#include "DataBaseManager.h"
#include "SourceFileParser.h"
#include "ParallelStrategyAlgorithmManager.h"
#include "ParserStatusManager.h"
#include "BaselineManagerService.h"
using namespace Dic::Module::Summary;
namespace Dic {
namespace Module {
namespace Global {
void BaselineManagerService::ResetBaseline()
{
    // 没有解析进程时才可以reset
    auto baselineId = Dic::Module::Timeline::BaselineManager::Instance().GetBaselineId();
    Dic::Module::Timeline::ParserStatusManager::Instance().WaitAllFinished({baselineId});
    Dic::Module::Timeline::DataBaseManager::Instance().ResetBaseline();
    BaselineManager::Instance().Reset();
    Source::SourceFileParser::Instance().ResetBaseline();
}

bool BaselineManagerService::CheckIsSupportCompare(const std::vector<ProjectExplorerInfo> &baseline,
                                                   const std::vector<ProjectExplorerInfo> &cur,
                                                   std::string &errorMsg, const std::string &filePath)
{
    if (baseline.empty() || cur.empty()) {
        return false;
    }
    // 多device场景不允许设置基线
    bool isAllSamePath = std::all_of(baseline[0].subParseFileInfo.begin(),
                                     baseline[0].subParseFileInfo.end(),
                                     [&filePath](const auto &fileInfo) {
                                         return fileInfo->parseFilePath == filePath;
                                     });
    if (baseline[0].subParseFileInfo.size() > 1 && isAllSamePath) {
        errorMsg = "Multi device scenario does not support setting comparison.";
        return false;
    }
    // MemScope不支持对比
    bool isBaselineMemScope = std::any_of(baseline[0].subParseFileInfo.begin(), baseline[0].subParseFileInfo.end(),
        [](const auto &fileInfo) {
            return std::regex_match(FileUtil::GetFileName(fileInfo->parseFilePath), std::regex(memScopeDbReg));
        });
    auto isCurMemScope = std::any_of(cur[0].subParseFileInfo.begin(), cur[0].subParseFileInfo.end(),
        [](const auto &fileInfo) {
            return std::regex_match(FileUtil::GetFileName(fileInfo->parseFilePath), std::regex(memScopeDbReg));
        });
    if (isBaselineMemScope || isCurMemScope) {
        errorMsg = "MemScope data does not support comparison function.";
        return false;
    }

    // 只有部分数据类型支持设置对比，如果非预期数据类型，则直接给前端返回错误提示
    auto projectTypeEnum = ProjectExplorerManager::GetProjectType(baseline);
    if (!IsSupportCompareType(projectTypeEnum)) {
        errorMsg = "Not supported to set the project type for baseline!";
        return false;
    }

    auto curProjectTypeEnum = ProjectExplorerManager::GetProjectType(cur);
    if (!IsSupportCompareType(curProjectTypeEnum)) {
        errorMsg = "The current project type does not support comparison function.";
        return false;
    }
    if (!IsComparable(projectTypeEnum, curProjectTypeEnum)) {
        errorMsg = "Comparison is not allowed for different data types.";
        return false;
    }
    return true;
}

bool BaselineManagerService::InitBaselineData(const Protocol::BaselineSettingRequest &request,
                                              BaselineInfo &baselineInfo)
{
    ResetBaseline();
    // 查询详细数据
    std::vector<std::string> filePathList;
    if (!request.params.filePath.empty()) {
        filePathList.push_back(request.params.filePath);
    }
    // 多集群场景下需要把所有的信息查出来
    std::vector<ProjectExplorerInfo> projectExplorerList =
        ProjectExplorerManager::Instance().QueryProjectExplorer(request.params.projectName, {});
    std::vector<ProjectExplorerInfo> curProject =
        ProjectExplorerManager::Instance().QueryProjectExplorer(request.projectName, {});
    if (projectExplorerList.empty() || curProject.empty()) {
        baselineInfo.errorMessage = "The project does not exist, baseline setting failed.";
        return false;
    }

    // 对于联合导入场景，需要准确找到子工程, 根据parseFilePath过滤，如果仍有多项，则以第一项为准
    auto projectFilter = [&request](const ProjectExplorerInfo &project) {
        bool isParseFile = std::any_of(project.subParseFileInfo.begin(),
                                       project.subParseFileInfo.end(),
                                       [&request](const auto fileInfo) {
                                           return fileInfo->parseFilePath == request.params.filePath;
                                       });
        auto clusterInfos = project.GetClusterInfos();
        bool isCluster = std::any_of(clusterInfos.begin(), clusterInfos.end(), [&request](const auto clusterInfo) {
            return clusterInfo->parseFilePath == request.params.baselineClusterPath;
        });
        isCluster |= (request.params.baselineClusterPath == request.projectName);
        return !(isParseFile || isCluster);
    };
    projectExplorerList.erase(
        std::remove_if(projectExplorerList.begin(), projectExplorerList.end(),
                       projectFilter),
        projectExplorerList.end()
    );
    // 检查是否支持对比，返回true是因为目前如果返回false则错误信息前端获取不到，返回false但errorMessage不为空，前端能正确识别到错误并提示
    if (!CheckIsSupportCompare(projectExplorerList, curProject, baselineInfo.errorMessage, request.params.filePath)) {
        return true;
    }
    auto projectTypeEnum = ProjectExplorerManager::GetProjectType(projectExplorerList);
    // 根据二级目录判断是否为集群数据
    baselineInfo.isCluster = IsClusterBaseline(projectTypeEnum, projectExplorerList, request.params.filePath);
    if (baselineInfo.isCluster) {
        BaselineManager::Instance().SetBaselineClusterPath(baselineInfo.clusterBaseLine);
    }
    // 获取解析类型（以进一步调用对应解析类）
    ParserType parserType = coverProjectTypeToParserType(projectTypeEnum);
    // 调用工厂进行内容解析
    std::shared_ptr<ProjectParserBase> parser = ParserFactory::GetProjectParser(parserType);
    parser->ParserBaseline(projectExplorerList[0], baselineInfo);
    // 集群场景 初始化并行策略
    if (baselineInfo.isCluster) {
        InitBaselineParallelStrategy(request.params.currentClusterPath);
    }
    return true;
}

bool BaselineManagerService::IsClusterBaseline(ProjectTypeEnum projectTypeEnum,
                                               const std::vector<ProjectExplorerInfo> &projectInfoList,
                                               const std::string &filePath)
{
    // 如果非text和db场景，则直接判断为非集群场景，返回false
    if (projectTypeEnum != ProjectTypeEnum::TEXT_CLUSTER && projectTypeEnum != ProjectTypeEnum::DB_CLUSTER) {
        return false;
    }
    bool isCluster = std::any_of(projectInfoList.begin(), projectInfoList.end(), [filePath](const auto &item) {
        auto cluster = item.GetClusterInfos();
        if (cluster.empty()) {
            return false;
        }
        return std::any_of(cluster.begin(), cluster.end(),
                           [&filePath](const std::shared_ptr<ParseFileInfo> &clusterInfo) {
                               return clusterInfo->parseFilePath == filePath;
                           });
    });
    if (isCluster) {
        return true;
    }
    // 整个项目当作集群
    if (!projectInfoList.empty() && !projectInfoList[0].projectFileTree.empty() &&
        projectInfoList[0].fileName == filePath) {
        return true;
    }
    return false;
}

void BaselineManagerService::InitBaselineParallelStrategy(const std::string &compareClusterPath)
{
    // 初始化baseline的并行策略，通过同步当前compare数据的并行策略
    auto database = FullDb::DataBaseManager::Instance().GetClusterDatabase(compareClusterPath);
    if (database == nullptr) {
        return;
    }
    auto config = ParallelStrategyAlgorithmManager::Instance().GetParallelStrategyConfig(database->GetDbPath());
    std::string errMsg;
    auto baselineDb = FullDb::DataBaseManager::Instance().GetClusterDatabase(
        BaselineManager::Instance().GetBaseLineClusterPath());
    if (baselineDb == nullptr) {
        return;
    }
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(baselineDb->GetDbPath(), config, errMsg);
}
}
}
}
