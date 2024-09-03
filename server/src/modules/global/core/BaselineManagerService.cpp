/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ProjectExplorerManager.h"
#include "ParserFactory.h"
#include "DataBaseManager.h"
#include "BaselineManager.h"
#include "BaselineManagerService.h"
namespace Dic {
namespace Module {
namespace Global {
void BaselineManagerService::ResetBaseline()
{
    Dic::Module::Timeline::DataBaseManager::Instance().ResetBaseline();
    BaselineManager::Instance().Reset();
}

bool BaselineManagerService::InitBaselineData(const std::string &projectName, const std::string &filePath,
    BaselineInfo &baselineInfo)
{
    ResetBaseline();
    // 查询详细数据
    std::vector<ProjectExplorerInfo> projectExplorerList =
        ProjectExplorerManager::Instance().QueryProjectExplorer(projectName, std::vector<std::string>{ filePath });
    if (projectExplorerList.empty() || projectExplorerList[0].parseFilePathInfos.empty()) {
        baselineInfo.errorMessage = "The project does not exist, baseline setting failed.";
        return false;
    }

    // 只有部分数据类型支持设置对比，如果非预期数据类型，则直接给前端返回错误提示
    auto projectTypeEnum = static_cast<ProjectTypeEnum>(projectExplorerList[0].projectType);
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
    std::shared_ptr<ParserAlloc> factory = ParserFactory::ParserImport(parserType);
    factory->ParserBaseline(projectExplorerList, baselineInfo);
    return true;
}
}
}
}
