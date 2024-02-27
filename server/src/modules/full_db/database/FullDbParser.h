/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_FULLDBPARSER_H
#define PROFILER_SERVER_FULLDBPARSER_H

#include "FileParser.h"
#include "ThreadPool.h"
#include "map"

namespace Dic::Module::FullDb {
class FullDbParser : public FileParser {
public:
    static FullDbParser &Instance();
    FullDbParser();

    ~FullDbParser() override;
    bool Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
               const std::string &selectedFolder) override;
    void Reset() override;
    static bool FindDevicePaths(const std::string &selectedFolder,
                                std::map<std::string, std::string>& devicePaths);
    static void InitOpenDb(const std::string &filePath, const std::string &rankId, const std::string& token);
    bool InitCluster(std::string path, std::string token);
private:
    std::unique_ptr<ThreadPool> threadPool;
    const int maxThreadNum = 4;

    static void InitMemory(std::vector<std::string> fileId, std::string path, std::string token);
    static void InitSummery(std::vector<std::string> fileId, std::string path, std::string token);

    static void ParserCallBack(std::string rankId, bool result);
};
}

#endif // PROFILER_SERVER_FULLDBPARSER_H
