/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_EXPERTHOTSPOTPARSER_H
#define PROFILER_SERVER_EXPERTHOTSPOTPARSER_H
#include <vector>
#include <string>
#include "ClusterDef.h"
#include "VirtualClusterDatabase.h"

namespace Dic::Module::Summary {
class ExpertHotspotParser {
public:
    explicit ExpertHotspotParser(std::shared_ptr<VirtualClusterDatabase> &database) : db(database) {}
    bool Parse(const std::string &filePath, const std::string &version);
    std::map<std::string, ModelInfo> GetModelInfoMap();
private:
    const static int regexMatchNumber = 3;
    std::shared_ptr<VirtualClusterDatabase> db = nullptr;
    std::map<std::string, ModelInfo> modelInfoMap;
};
}


#endif // PROFILER_SERVER_EXPERTHOTSPOTPARSER_H
