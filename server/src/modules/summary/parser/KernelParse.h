/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_KERNELPARSE_H
#define PROFILER_SERVER_KERNELPARSE_H

#include <string>
#include <map>
#include <functional>
#include "SummaryDef.h"
#include "ThreadPool.h"
#include "SummaryDataBase.h"
#include "FileParser.h"

namespace Dic {
namespace Module {
namespace Summary {
class KernelParse : public FileParser {
public:
    static KernelParse &Instance();
    KernelParse();
    ~KernelParse() override;
    void Reset() override;

    bool Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
               const std::string &selectedFolder) override;
    bool KernelFileParse(const std::string &filePath, const std::string &fileId);
private:
    std::unique_ptr<SummaryDataBase> database;
    const int maxThreadNum = 1;
    std::unique_ptr<ThreadPool> threadPool;
    std::chrono::system_clock::time_point start;
    std::map<std::string, std::future<void>> futureMap;
    const std::string kernelDetailFile = "kernel_details.csv";

    Kernel mapperToKernelDetail(std::map<std::string, int16_t> dataMap, std::vector<std::string> row);

    bool WaitParseEnd(const std::string &fileId);
};

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_KERNELPARSE_H
