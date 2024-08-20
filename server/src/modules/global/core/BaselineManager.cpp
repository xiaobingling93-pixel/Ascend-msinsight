/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "BaselineManager.h"
#include "ProjectExplorerManager.h"
#include "ParserFactory.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Global {
BaselineManager &BaselineManager::Instance()
{
    static BaselineManager instance;
    return instance;
}

bool BaselineManager::IsSelectBaseline()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (parseFileId < 0) {
        return false;
    }
    return true;
}

void BaselineManager::ResetBaseline()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (parseFileId < 0) {
        return;
    }
    FullDb::DataBaseManager::Instance().ResetBaseline();
    parseFileId = -1;
    return;
}

bool BaselineManager::InitBaselineData(const std::string &projectName, const std::string &filePath,
                                       std::string &errorMsg, std::string &rankId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    ResetBaseline();
    // 查询详细数据
    std::vector<ProjectExplorerInfo> projectExplorerList = ProjectExplorerManager::Instance().QueryProjectExplorer(
        projectName, std::vector<std::string>{filePath});
    if (projectExplorerList.empty() || projectExplorerList[0].parseFilePathInfos.empty()) {
        errorMsg = "The project does not exist, baseline setting failed.";
        return false;
    }

    // 只有部分数据类型支持设置对比，如果非预期数据类型，则直接给前端返回错误提示
    auto projectTypeEnum = static_cast<ProjectTypeEnum>(projectExplorerList[0].projectType);
    // todo-yqs 将类型判断加回来
    parseFileId = projectExplorerList[0].parseFilePathInfos[0].id;
    rankId = baselineMark + std::to_string(parseFileId);
    // 获取解析类型（以进一步调用对应解析类）
    ParserType parserType = coverProjectTypeToParserType(projectTypeEnum);
    // 设置baseline数据库的类型
    Timeline::DataType type = parserType == ParserType::DB ? Timeline::DataType::DB : Timeline::DataType::TEXT;
    Timeline::DataBaseManager::Instance().SetBaselineDataType(type);
    // 调用工厂进行内容解析
    std::shared_ptr<ParserAlloc> factory = ParserFactory::ParserImport(parserType);
    factory->ParserBaseline(projectExplorerList, rankId);
    return true;
}

std::string BaselineManager::GetBaselineId()
{
    return baselineMark + std::to_string(parseFileId);
}

bool BaselineManager::IsBaselineId(const string &rankId)
{
    if (rankId.empty()) {
        return false;
    }
    return StringUtil::Contains(rankId, baselineMark);
}

}
}
}