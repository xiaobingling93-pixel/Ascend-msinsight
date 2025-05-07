/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ExpertHotspotManager.h"
#include "ExpertHotspotParser.h"
#include "ExpertDeploymentParser.h"
#include "DataBaseManager.h"
#include "ClusterDef.h"
#include "CollectionUtil.h"
#include "NumberSafeUtil.h"

namespace Dic {
namespace Module {
namespace Summary {

std::map<std::string, ModelInfo> ExpertHotspotManager::ParseHotspotData(const std::vector<std::string> &hotspotFiles,
    std::string &errorMsg, std::shared_ptr<VirtualClusterDatabase> &database, const std::string version)
{
    // 清空老数据
    if (!database->DeleteExpertHotspot("", version)) {
        errorMsg = "Failed to clear old expert hotspot data, version:" + version;
        return {};
    }
    ExpertHotspotParser parser(database);
    for (const auto &item: hotspotFiles) {
        // 文件解析，单个文件解析失败不影响最终结果
        if (!parser.Parse(item, version)) {
            ServerLog::Warn("Fail to parser file:", item);
        }
    }
    database->SaveExpertHotspot();
    return parser.GetModelInfoMap();
}

std::map<std::string, ModelInfo> ExpertHotspotManager::ParseDeploymentData(
    const std::vector<std::string> &deploymentFiles, std::string &errorMsg,
    std::shared_ptr<VirtualClusterDatabase> &database, const std::string version)
{
    // 清空老数据
    if (!database->DeleteDeployment("", version)) {
        errorMsg = "Failed to clear old expert deployment data, version:" + version;
        return {};
    }
    // 文件解析
    ExpertDeploymentParser parser(database);
    for (const auto &item: deploymentFiles) {
        if (!parser.Parse(item, version)) {
            ServerLog::Warn("Fail to parser file:", item);
        }
    }
    database->SaveExpertDeployment();
    return parser.GetModelInfoMap();
}

bool ExpertHotspotManager::MergeAndSaveModelInfo(const std::map<std::string, ModelInfo> &hotspotModelInfo,
    const std::map<std::string, ModelInfo> &deploymentModelInfo, std::shared_ptr<VirtualClusterDatabase> &database)
{
    // 获取当前db中已保存的modelInfo信息,稠密层列表从老数据中直接保留，模型总层数取当前值和新导入数据的最大值
    ModelInfo curModelInfo = GetModelInfo(database);
    ModelInfo hotspot = hotspotModelInfo.empty() ? ModelInfo() : hotspotModelInfo.begin()->second;
    ModelInfo deployment = deploymentModelInfo.empty() ? ModelInfo() : deploymentModelInfo.begin()->second;
    bool isAllExist = !hotspotModelInfo.empty() && !deploymentModelInfo.empty();
    bool isParamsNotEqual = hotspot.moeLayer != deployment.moeLayer || hotspot.rankNumber != deployment.rankNumber;
    if (isAllExist && isParamsNotEqual) {
        return false;
    }
    ModelInfo finalModelInfo;
    finalModelInfo.denseLayerList = curModelInfo.denseLayerList;
    finalModelInfo.modelLayer = curModelInfo.modelLayer;
    // 配置信息保存
    if (!hotspotModelInfo.empty()) {
        // 计算总层数，从以下两个数据中取大的数据：1.导入数据的moe层数+已配置的稠密层数；2.已配置的总层数
        int totalModelLayer = NumberUtil::CeilingClamp(
            NumberSafe::Add(hotspot.moeLayer, curModelInfo.denseLayerList.size()), static_cast<uint64_t>(INT_MAX));
        finalModelInfo.modelLayer = std::max(totalModelLayer, finalModelInfo.modelLayer);
        finalModelInfo.rankNumber = hotspot.rankNumber;
        finalModelInfo.moeLayer = hotspot.moeLayer;
        finalModelInfo.expertNumber = hotspot.expertNumber;
    }

    if (!deploymentModelInfo.empty()) {
        int totalModelLayer = NumberUtil::CeilingClamp(
            NumberSafe::Add(deployment.moeLayer, curModelInfo.denseLayerList.size()), static_cast<uint64_t>(INT_MAX));
        finalModelInfo.modelLayer = std::max(totalModelLayer, finalModelInfo.modelLayer);
        finalModelInfo.rankNumber = deployment.rankNumber;
        finalModelInfo.moeLayer = deployment.moeLayer;
        finalModelInfo.expertNumber = deployment.expertNumber;
    }

    return SaveModelInfo(finalModelInfo, database);
}

bool ExpertHotspotManager::InitExpertHotspotData(const std::string &filePath, const std::string &version,
                                                 std::string &errorMsg)
{
    // 参数校验，ConvertToRealPath方法中会调用CheckDirValid方法对文件进行校验
    std::string realFilePath = filePath;
    if (!FileUtil::ConvertToRealPath(errorMsg, realFilePath)) {
        return false;
    }
    // 获取db
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr) {
        errorMsg = "Cluster database is not exist.";
        return false;
    }

    // 查找文件列表
    auto hotspotFiles = FileUtil::FindAllFilesByRegex(realFilePath, std::regex(expertHotspotFileReg));
    auto deploymentFiles = FileUtil::FindAllFilesByRegex(realFilePath, std::regex(expertDeploymentFileReg));
    if (hotspotFiles.empty() && deploymentFiles.empty()) {
        errorMsg = "No parsable files found";
        return false;
    }

    // 文件内容解析并落库
    auto hotspotModelInfo = ParseHotspotData(hotspotFiles, errorMsg, database, version);
    auto deploymentModelInfo = ParseDeploymentData(deploymentFiles, errorMsg, database, version);
    if (!errorMsg.empty()) {
        return false;
    }
    // 配置信息保存
    return MergeAndSaveModelInfo(hotspotModelInfo, deploymentModelInfo, database);
}

bool ExpertHotspotManager::SaveModelInfo(const ModelInfo &modelInfo, std::shared_ptr<VirtualClusterDatabase> &db)
{
    std::map<std::string, std::string> modelInfoMap;
    modelInfoMap[KEY_DENSE_LAYER_LIST] = StringUtil::join(modelInfo.denseLayerList, ",");
    // 只有数据不为0才展示
    if (modelInfo.moeLayer != 0) {
        modelInfoMap[KEY_MOE_LAYER] = std::to_string(modelInfo.moeLayer);
    }
    if (modelInfo.rankNumber != 0) {
        modelInfoMap[KEY_RANK_NUMBER] = std::to_string(modelInfo.rankNumber);
    }
    if (modelInfo.expertNumber != 0) {
        modelInfoMap[KEY_EXPERT_NUMBER] = std::to_string(modelInfo.expertNumber);
    }
    if (modelInfo.modelLayer != 0) {
        modelInfoMap[KEY_MODEL_LAYER] = std::to_string(modelInfo.modelLayer);
    }
    return db->InsertDuplicateUpdateBaseInfo(modelInfoMap);
}

bool ExpertHotspotManager::UpdateModelInfo(ModelInfo &newModelInfo, std::string &errorMsg)
{
    // 获取db
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr) {
        errorMsg = "Fail to update model info, database not exist.";
        return false;
    }
    ModelInfo curModelInfo = GetModelInfo(database);
    // 专家数量不允许修改
    if (curModelInfo.expertNumber != 0 && curModelInfo.expertNumber != newModelInfo.expertNumber) {
        errorMsg = "Fail to update model info, the number of expert number can't be modify.";
        return false;
    }
    if (newModelInfo.modelLayer - curModelInfo.moeLayer < newModelInfo.denseLayerList.size()) {
        errorMsg = "Fail to update model info, "
                   "the sum of moe and dense layers is less than the total number of layers in the model.";
        return false;
    }
    curModelInfo.modelLayer = newModelInfo.modelLayer;
    curModelInfo.expertNumber = newModelInfo.expertNumber;
    curModelInfo.denseLayerList = newModelInfo.denseLayerList;
    return SaveModelInfo(curModelInfo, database);
}

ModelInfo ExpertHotspotManager::GetModelInfo()
{
     // 获取db
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr) {
        ServerLog::Error("Fail to get model info, database not exist.");
        return {};
    }
    return GetModelInfo(database);
}

ModelInfo ExpertHotspotManager::GetModelInfo(std::shared_ptr<VirtualClusterDatabase> &db)
{
    std::vector<std::string> keys = {KEY_DENSE_LAYER_LIST, KEY_MOE_LAYER, KEY_RANK_NUMBER, KEY_EXPERT_NUMBER,
                                     KEY_MODEL_LAYER};
    std::map<std::string, std::string> modelInfoMap = db->QueryBaseInfoByKeys(keys);
    std::string defaultZeroStr = "0";
    ModelInfo modelInfo;
    std::string denseLayerListStr = CollectionUtil::FindValueByKey(modelInfoMap, KEY_DENSE_LAYER_LIST,
                                                                   CollectionUtil::EMPTY_STRING);
    if (!denseLayerListStr.empty()) {
        for (const auto &item: StringUtil::Split(denseLayerListStr, ",")) {
            modelInfo.denseLayerList.push_back(StringUtil::StringToInt(item));
        }
    }
    modelInfo.expertNumber = StringUtil::StringToInt(
        CollectionUtil::FindValueByKey(modelInfoMap, KEY_EXPERT_NUMBER, defaultZeroStr));
    modelInfo.rankNumber = StringUtil::StringToInt(
        CollectionUtil::FindValueByKey(modelInfoMap, KEY_RANK_NUMBER, defaultZeroStr));
    modelInfo.moeLayer = StringUtil::StringToInt(
        CollectionUtil::FindValueByKey(modelInfoMap, KEY_MOE_LAYER, defaultZeroStr));
    modelInfo.modelLayer = StringUtil::StringToInt(
        CollectionUtil::FindValueByKey(modelInfoMap, KEY_MODEL_LAYER, defaultZeroStr));
    return modelInfo;
}

int ExpertHotspotManager::CalColumnNumber(const std::vector<ExpertHotspotStruct> &hotspotInfos,
    const ModelInfo &modelInfo, const std::vector<ExpertDeploymentStruct> &deployment)
{
    int colNumber = 0;
    if (!hotspotInfos.empty() && modelInfo.moeLayer > 0 && hotspotInfos.size() % modelInfo.moeLayer == 0) {
        // 如果热点数据存在，则：列数 = 热点数据总个数 / moe层数，需要保证moe层数大于0，并且能够整除，否则代表数据不正常
        colNumber = hotspotInfos.size() / modelInfo.moeLayer;
    } else if (!deployment.empty() && modelInfo.rankNumber != 0) {
        // 热点数据不存在，但是配置文件存在，则 列数 = 每个rank专家数 * rank数量
        colNumber = deployment[0].expertList.size() * modelInfo.rankNumber;
    } else {
        // 如果热点数据不存在，则使用专家个数作为列数
        colNumber = modelInfo.expertNumber;
    }
    return colNumber;
}

std::vector<int> ExpertHotspotManager::CalMoeLayerMapping(const ModelInfo &modelInfo,
                                                          const std::set<int> &denseLayerSet)
{
    // 计算从moe层映射到整体层的映射
    std::vector<int> moeLayerMapping(modelInfo.moeLayer);
    int moeLayerIndex = 0;
    for (size_t i = 0; i < modelInfo.modelLayer; ++i) {
        if (moeLayerIndex >= modelInfo.moeLayer) {
            break;
        }
        if (denseLayerSet.find(i) != denseLayerSet.end()) {
            continue;
        }
        moeLayerMapping[moeLayerIndex++] = i;
    }
    return moeLayerMapping;
}

bool ExpertHotspotManager::FillHotspotData(std::vector<ExpertHotspotStruct> &res, FillExpertDataParams &params)
{
    for (auto &item: params.hotspotInfos) {
        if (item.layer >= params.modelInfo.moeLayer) {
            ServerLog::Error("Invalid hotspot data.");
            return false;
        }
        // 更新layer为全局layer（包含稠密层的情况）
        item.layer = params.moeLayerMapping[item.layer];
        item.expertId = NumberSafe::Add(NumberSafe::Muls(item.rankId, params.expertNumberPerRank), item.localExpertId);
        item.expertIndex =
            NumberSafe::Add(NumberSafe::Muls(item.rankId, params.expertNumberPerRank), item.localExpertId);
        int index = NumberSafe::Add(item.expertIndex, NumberSafe::Muls(item.layer, params.colNumber));
        if (index > NumberSafe::Muls(params.colNumber, params.modelInfo.modelLayer)) {
            return false;
        }
        res[index] = item;
    }
    return true;
}

bool ExpertHotspotManager::FillDeploymentData(std::vector<ExpertHotspotStruct> &res, FillExpertDataParams &params)
{
    for (const auto &item: params.deployment) {
        if (item.layer >= params.modelInfo.moeLayer) {
            ServerLog::Error("Invalid deployment data.");
            return false;
        }
        int aclLayer = params.moeLayerMapping[item.layer];
        for (size_t i = 0; i < item.expertList.size(); ++i) {
            int expertIndex = NumberSafe::Add(i, NumberSafe::Muls(params.expertNumberPerRank, item.deviceId));
            int index =  NumberSafe::Add(expertIndex, NumberSafe::Muls(aclLayer, params.colNumber));
            if (index > NumberSafe::Muls(params.colNumber, params.modelInfo.modelLayer)) {
                return false;
            }
            res[index].expertIndex = expertIndex;
            res[index].layer = aclLayer;
            res[index].expertId = item.expertList[i];
            res[index].rankId = item.deviceId;
        }
    }
    return true;
}

void ExpertHotspotManager::FillDenseLayerInfo(std::vector<ExpertHotspotStruct> &res, FillExpertDataParams &params)
{
    // 空内容填充：空内容包含两部分，1.设置为稠密层的数据；2.总层数中最后几层未被稠密层和热点数据覆盖到的数据
    if (!params.moeLayerMapping.empty() && !params.hotspotInfos.empty()) {
        int index = params.moeLayerMapping.size() - 1;
        for (int i = params.moeLayerMapping[index] + 1; i < params.modelInfo.modelLayer; ++i) {
            params.denseLayerSet.insert(i);
        }
    } else {
        // 如果没有没有moe层 也就是说所有层都是空层
        for (size_t i = 0; i < params.modelInfo.modelLayer; ++i) {
            params.denseLayerSet.insert(i);
        }
    }
    for (const auto &item: params.denseLayerSet) {
        int startIndex = item * params.colNumber;
        for (int i = 0; i < params.colNumber; ++i) {
            int index = startIndex + i;
            res[index].expertIndex = i;
            res[index].layer = item;
        }
    }
}

bool ExpertHotspotManager::FillExpertInfo(std::vector<ExpertHotspotStruct> &hotspotInfos,
    const ModelInfo &modelInfo, const std::vector<ExpertDeploymentStruct> &deployment)
{
    // 校验并计算结果列表长度，行数为模型总层数（已知），列数需要根据情况来计算
    int colNumber = CalColumnNumber(hotspotInfos, modelInfo, deployment);
    if (colNumber == 0 || modelInfo.modelLayer == 0) {
        return false;
    }
    // 获取每个rank的专家数，这里如果未导入热点数据或配置数据，则rank数量位置，该参数默认为0
    int expertNumberPerRank = 0;
    if (!hotspotInfos.empty()) {
        if (modelInfo.rankNumber == 0 || colNumber % modelInfo.rankNumber != 0) {
            ServerLog::Error("Invalid model info: Rank number cannot be zero, "
                             "and expert number should be divisible by rank number.");
            return false;
        }
        // 如果有热点数据，则 专家数 = 列数 / rank数
        expertNumberPerRank = colNumber / modelInfo.rankNumber;
    } else if (!deployment.empty()) {
        // 如果有配置文件，则专家数直接取自数据内
        expertNumberPerRank = deployment[0].expertList.size();
    }
    // 初始化结果列表
    std::vector<ExpertHotspotStruct> res(colNumber * modelInfo.modelLayer);
    std::set<int> denseLayerSet(modelInfo.denseLayerList.begin(), modelInfo.denseLayerList.end());

    // 计算从moe层映射到整体层的映射
    std::vector<int> moeLayerMapping = CalMoeLayerMapping(modelInfo, denseLayerSet);

    // 热点数据填充，如果热点数据不存在，会直接返回true，不阻塞整体流程
    FillExpertDataParams params{modelInfo, hotspotInfos, deployment, moeLayerMapping,
                                colNumber, expertNumberPerRank, denseLayerSet};
    if (!FillHotspotData(res, params)) {
        return false;
    }

    if (!FillDeploymentData(res, params)) {
        return false;
    }

    // 空内容填充：空内容包含两部分，1.设置为稠密层的数据；2.总层数中最后几层未被稠密层和热点数据覆盖到的数据
    FillDenseLayerInfo(res, params);
    hotspotInfos = res;
    return true;
}

std::vector<ExpertHotspotStruct> ExpertHotspotManager::QueryExpertHotspotData(const std::string &modelStage,
                                                                              const std::string &version)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr) {
        return {};
    }
    auto hotspotRes = database->QueryExpertHotspotData(modelStage, version);
    auto deploymentRes = database->QueryExpertDeployment(modelStage, version);
    auto modelInfo = GetModelInfo(database);
    if (!FillExpertInfo(hotspotRes, modelInfo, deploymentRes)) {
        return {};
    }
    return hotspotRes;
}
}
}
}