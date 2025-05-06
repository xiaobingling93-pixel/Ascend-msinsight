/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_EXPERTDEPLOYMENTPARSER_H
#define PROFILER_SERVER_EXPERTDEPLOYMENTPARSER_H
#include <string>
#include <map>
#include "ClusterDef.h"
#include "VirtualClusterDatabase.h"

namespace Dic::Module::Summary {
class ExpertDeploymentParser {
public:
    explicit ExpertDeploymentParser(std::shared_ptr<VirtualClusterDatabase> &database) : db(database) {}
    bool Parse(const std::string &filePath, const std::string &version);
    std::map<std::string, ModelInfo> GetModelInfoMap();
private:
    const static int regexMatchNumber = 2;
    std::shared_ptr<VirtualClusterDatabase> db = nullptr;
    std::map<std::string, ModelInfo> modelInfoMap;
};
}
#endif // PROFILER_SERVER_EXPERTDEPLOYMENTPARSER_H
