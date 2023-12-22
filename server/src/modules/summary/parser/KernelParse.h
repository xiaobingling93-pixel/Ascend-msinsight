/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_KERNELPARSE_H
#define PROFILER_SERVER_KERNELPARSE_H

#include <string>
#include <map>
#include <set>
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
    void KernelFileParse(const std::string &parentDir, const std::string &fileId, std::set<std::string> &devices);
private:
    std::unique_ptr<SummaryDataBase> database;
    const int maxThreadNum = 4;
    const int kernelTableNum = 15;
    std::unique_ptr<ThreadPool> threadPool;
    std::chrono::system_clock::time_point start;
    std::map<std::string, std::future<void>> futureMap;
    const std::string kernelDetailFile = "kernel_details.csv";
    const std::string kernelDetailReg = R"((kernel_details|op_summary_[0-9]{1,14})\.csv$)";

    bool mapperToKernelDetail(std::map<std::string, int16_t> dataMap,
                              std::vector<std::string> row, const std::string &fileId, Kernel &kernel);

    bool WaitParseEnd(const std::string &fileId);
    std::vector<std::string> StringSplit(const std::string& str);
};

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_KERNELPARSE_H
