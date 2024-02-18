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
#include "ConstantDefs.h"

namespace Dic {
namespace Module {
namespace Summary {
// 表头字段-公共部分
const std::string FIELD_BLOCK_DIM = "Block Dim";
const std::string FIELD_INPUT_SHAPES = "Input Shapes";
const std::string FIELD_INPUT_DATA_TYPES = "Input Data Types";
const std::string FIELD_INPUT_FORMATS = "Input Formats";
const std::string FIELD_OUTPUT_SHAPES = "Output Shapes";
const std::string FIELD_OUTPUT_DATA_TYPES = "Output Data Types";
const std::string FIELD_OUTPUT_FORMATS = "Output Formats";

// 表头字段-Ascend PyTorch Profiler
const std::string FIELD_NAME = "Name";
const std::string FIELD_TYPE = "Type";
const std::string FIELD_ACCELERATOR_CORE = "Accelerator Core";
const std::string FIELD_START_TIME = "Start Time(us)";
const std::string FIELD_DURATION = "Duration(us)";
const std::string FIELD_WAIT_TIME = "Wait Time(us)";

// 表头字段-msprof
const std::string FIELD_OP_NAME = "Op Name";
const std::string FIELD_OP_TYPE = "OP Type";
const std::string FIELD_TASK_TYPE = "Task Type";
const std::string FIELD_TASK_START_TIME = "Task Start Time(us)";
const std::string FIELD_TASK_DURATION = "Task Duration(us)";
const std::string FIELD_TASK_WAIT_TIME = "Task Wait Time(us)";

class KernelParse : public FileParser {
public:
    static KernelParse &Instance();
    KernelParse();
    ~KernelParse() override;
    void Reset() override;

    bool Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
               const std::string &selectedFolder) override;
    bool Parse(const std::vector<std::string>& pathList, const std::string& token);

private:
    const int maxThreadNum = 4;
    static const int kernelTableNum = 15;
    std::unique_ptr<ThreadPool> threadPool;
    std::map<std::string, std::future<void>> futureMap;
    const std::string kernelDetailReg = R"((kernel_details|op_summary[_\d]*)\.csv$)";

    static bool mapperToKernelDetail(std::map<std::string, size_t> dataMap,
                              std::vector<std::string> row, const std::string &fileId, Kernel &kernel);

    std::vector<std::pair<std::string, std::string>> GetKernelFiles(const std::vector<std::string>& paths);

    static bool IsFileValid(const std::string &filePath, const std::string &fileId, const std::string& statusId,
                            std::string &message);
    static void SetParseCallBack(const std::string& token);
    static void ParseEndCallBack(const std::string& fileId, bool result, const std::string &msg);
    static void ParseCallBack(const std::string &token, const std::string& fileId, bool result, const std::string &msg);

    static void PreParseTask(const std::string &filePath, const std::string &fileId);
    static bool ParseTask(const std::string &filePath, const std::string &fileId, std::string &message);
    static bool InitParser(const std::string &filePath, const std::string &fileId, std::string &message);
};

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_KERNELPARSE_H
