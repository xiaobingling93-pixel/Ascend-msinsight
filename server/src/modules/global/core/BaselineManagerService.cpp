/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ProjectExplorerManager.h"
#include "ProjectParserFactory.h"
#include "DataBaseManager.h"
#include "BaselineManager.h"
#include "SourceFileParser.h"
#include "ClusterFileParser.h"
#include "ParallelStrategyAlgorithmManager.h"
#include "MegatronParallelStrategyAlgorithm.h"
#include "MindSpeedParallelStrategyAlgorithm.h"
#include "BaselineManagerService.h"
using namespace Dic::Module::Summary;
namespace Dic {
namespace Module {
namespace Global {
void BaselineManagerService::ResetBaseline()
{
    Dic::Module::Timeline::DataBaseManager::Instance().ResetBaseline();
    BaselineManager::Instance().Reset();
    Source::SourceFileParser::Instance().ResetBaseline();
}

bool BaselineManagerService::InitBaselineData(const std::string &projectName, const std::string &filePath,
    BaselineInfo &baselineInfo)
{
    ResetBaseline();
    // 查询详细数据
    std::vector<std::string> filePathList;
    if (!filePath.empty()) {
        filePathList.push_back(filePath);
    }
    std::vector<ProjectExplorerInfo> projectExplorerList =
        ProjectExplorerManager::Instance().QueryProjectExplorer(projectName, filePathList);
    if (projectExplorerList.empty() || projectExplorerList[0].subParseFileInfo.empty()) {
        baselineInfo.errorMessage = "The project does not exist, baseline setting failed.";
        return false;
    }

    // 只有部分数据类型支持设置对比，如果非预期数据类型，则直接给前端返回错误提示
    auto projectTypeEnum = ProjectExplorerManager::GetProjectType(projectExplorerList);
    // 根据二级目录判断是否为集群数据
    baselineInfo.isCluster = IsClusterBaseline(projectTypeEnum, filePath);
    if (!IsBaseLineConfigurableType(projectTypeEnum)) {
        baselineInfo.errorMessage = "Not supported to set the project type for baseline!";
        return false;
    }
    // 获取解析类型（以进一步调用对应解析类）
    ParserType parserType = coverProjectTypeToParserType(projectTypeEnum);
    // 设置baseline数据库的类型
    Timeline::DataType type = parserType == ParserType::DB ? Timeline::DataType::DB : Timeline::DataType::TEXT;
    Timeline::DataBaseManager::Instance().SetBaselineDataType(type);
    // 调用工厂进行内容解析
    std::shared_ptr<ProjectParserBase> factory = ParserFactory::ParserImport(parserType);
    factory->ParserBaseline(projectExplorerList, baselineInfo);
    // 集群场景 初始化并行策略
    if (baselineInfo.isCluster) {
        InitBaselineParallelStrategy();
    }
    return true;
}

bool BaselineManagerService::IsClusterBaseline(ProjectTypeEnum projectTypeEnum, const std::string &fileName)
{
    // 如果非text和db场景，则直接判断为非集群场景，返回false
    if (projectTypeEnum != ProjectTypeEnum::TEXT_CLUSTER && projectTypeEnum != ProjectTypeEnum::DB_CLUSTER) {
        return false;
    }
    // 如果内容为空，即说明设置的是一级目录的对比，视为集群对比
    if (fileName.empty()) {
        return true;
    }
    // 校验是否为集群数据
    return FullDb::ClusterFileParser::CheckIsCluster(fileName);
}

void BaselineManagerService::InitBaselineParallelStrategy()
{
    // 初始化baseline的并行策略，通过同步当前compare数据的并行策略
    auto database = FullDb::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr) {
        return;
    }
    auto config = ParallelStrategyAlgorithmManager::Instance().GetParallelStrategyConfig(database->GetDbPath());
    std::string errMsg;
    auto baselineDb = FullDb::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
    if (baselineDb == nullptr) {
        return;
    }
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(baselineDb->GetDbPath(), config, errMsg);
}
}
}
}
