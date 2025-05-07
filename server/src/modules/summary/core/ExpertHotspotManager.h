/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_EXPERTHOTSPOTMANAGER_H
#define PROFILER_SERVER_EXPERTHOTSPOTMANAGER_H
#include <string>
#include "ClusterDef.h"
#include "VirtualClusterDatabase.h"
namespace Dic {
namespace Module {
namespace Summary {
struct FillExpertDataParams {
    ModelInfo modelInfo;
    std::vector<ExpertHotspotStruct> hotspotInfos;
    std::vector<ExpertDeploymentStruct> deployment;
    std::vector<int> moeLayerMapping;
    int colNumber = 0;
    int expertNumberPerRank = 0;
    std::set<int> denseLayerSet;
};
class ExpertHotspotManager {
public:
    static bool InitExpertHotspotData(const std::string &filePath, const std::string &version, std::string &errorMsg);
    static std::vector<ExpertHotspotStruct> QueryExpertHotspotData(const std::string &modelStage,
                                                                    const std::string &version);
    static ModelInfo GetModelInfo();
    static bool UpdateModelInfo(ModelInfo &newModelInfo, std::string &errorMsg);
private:
    static bool FillExpertInfo(std::vector<ExpertHotspotStruct> &hotspotInfos, const ModelInfo &modelInfo,
                               const std::vector<ExpertDeploymentStruct> &deployment);
    static std::map<std::string, ModelInfo> ParseHotspotData(const std::vector<std::string> &hotspotFiles,
        std::string &errorMsg, std::shared_ptr<VirtualClusterDatabase> &database, const std::string version);
    static std::map<std::string, ModelInfo> ParseDeploymentData(const std::vector<std::string> &deploymentFiles,
        std::string &errorMsg, std::shared_ptr<VirtualClusterDatabase> &database, const std::string version);
    static bool MergeAndSaveModelInfo(const std::map<std::string, ModelInfo> &hotspotModelInfo,
        const std::map<std::string, ModelInfo> &deploymentModelInfo, std::shared_ptr<VirtualClusterDatabase> &database);
    static bool SaveModelInfo(const ModelInfo &modelInfo, std::shared_ptr<VirtualClusterDatabase> &db);
    static ModelInfo GetModelInfo(std::shared_ptr<VirtualClusterDatabase> &db);
    static int CalColumnNumber(const std::vector<ExpertHotspotStruct> &hotspotInfos, const ModelInfo &modelInfo,
                               const std::vector<ExpertDeploymentStruct> &deployment);
    static std::vector<int> CalMoeLayerMapping(const ModelInfo &modelInfo, const std::set<int> &denseLayerSet);
    static bool FillHotspotData(std::vector<ExpertHotspotStruct> &res, FillExpertDataParams &params);
    static bool FillDeploymentData(std::vector<ExpertHotspotStruct> &res, FillExpertDataParams &params);
    static void FillDenseLayerInfo(std::vector<ExpertHotspotStruct> &res, FillExpertDataParams &params);
};
}
}
}
#endif // PROFILER_SERVER_EXPERTHOTSPOTMANAGER_H
