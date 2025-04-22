/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_EXPERTHOTSPOTMANAGER_H
#define PROFILER_SERVER_EXPERTHOTSPOTMANAGER_H
#include <string>
#include "ClusterDef.h"
namespace Dic {
namespace Module {
namespace Summary {
class ExpertHotspotManager {
public:
    static bool InitExpertHotspotData(const std::string &filePath, const std::string &version, std::string &errorMsg);
    static std::vector<ExpertHotspotStruct> QueryExpertHotsSpotData(const std::string &modelStage,
                                                                    const std::string &version);
};
}
}
}
#endif // PROFILER_SERVER_EXPERTHOTSPOTMANAGER_H
