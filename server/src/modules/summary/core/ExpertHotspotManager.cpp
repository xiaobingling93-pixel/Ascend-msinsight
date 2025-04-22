/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ExpertHotspotManager.h"
#include "FileUtil.h"
#include "ExpertHotspotParser.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Summary {
bool ExpertHotspotManager::InitExpertHotspotData(const std::string &filePath, const std::string &version,
                                                 std::string &errorMsg)
{
    // 参数校验，ConvertToRealPath方法中会调用CheckDirValid方法对文件进行校验
    std::string realFilePath = filePath;
    if (!FileUtil::ConvertToRealPath(errorMsg, realFilePath)) {
        return false;
    }
    // 查找文件列表
    auto hotspotFiles = FileUtil::FindAllFilesByRegex(realFilePath, std::regex(ExpertHotspotFileReg));
    if (hotspotFiles.size() == 0) {
        errorMsg = "No parsable files found";
        return false;
    }
    // 获取db
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr) {
        errorMsg = "Cluster database is not exist.";
        return false;
    }
    // 清空老数据
    if (!database->DeleteExpertHotspot("", version)) {
        errorMsg = "Failed to clear old expert hotspot data, version:" + version;
        return false;
    }
    ExpertHotspotParser parser(database);
    for (const auto &item: hotspotFiles) {
        // 文件解析，单个文件解析失败不影响最终结果
        if (!parser.Parser(item, version)) {
            ServerLog::Warn("Fail to parser file:", item);
        }
    }
    database->SaveExpertHotspot();
    return true;
}

std::vector<ExpertHotspotStruct> ExpertHotspotManager::QueryExpertHotsSpotData(const std::string &modelStage,
                                                                               const std::string &version)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr) {
        return {};
    }
    auto hotspotRes = database->QueryExpertHotspotData(modelStage, version);
    return hotspotRes;
}
}
}
}