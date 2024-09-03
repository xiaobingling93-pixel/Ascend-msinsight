/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_FILEMENUDATABASEDEF_H
#define PROFILER_SERVER_FILEMENUDATABASEDEF_H

#include <string>
#include <vector>

namespace Dic {
namespace Module {
namespace Global {
struct ParseFileInfo {
    int64_t id = -1;
    int64_t projectExplorerId = -1;
    std::string parseFilePath;
    std::string dbPath;
};

struct ProjectExplorerInfo {
    int64_t id = -1;
    std::string projectName;
    std::string fileName;
    std::vector<ParseFileInfo> parseFilePathInfos;
    int64_t projectType = 0;
    std::string importType;
    std::vector<std::string> dbPath;
};

struct BaselineInfo {
    std::string host;
    std::string rankId;
    std::string cardName;
    std::string errorMessage;
};
}
}
}
#endif // PROFILER_SERVER_FILEMENUDATABASEDEF_H
