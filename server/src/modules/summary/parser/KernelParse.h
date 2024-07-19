/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_KERNELPARSE_H
#define PROFILER_SERVER_KERNELPARSE_H

#include <string>
#include <map>
#include <set>
#include <functional>
#include "SummaryDef.h"
#include "ThreadPool.h"
#include "JsonSummaryDataBase.h"
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

const std::string ASCEND_PYTORCH_PROF = "AscendPyTorchprof";
const std::string MSPROF = "Msprof";
const std::string L1 = "L1";
const std::string L0 = "L0";

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
    std::unique_ptr<ThreadPool> threadPool;
    std::map<std::string, std::future<void>> futureMap;
    static std::map<std::string, std::function<void(const std::map<std::string, size_t> &dataMap,
            const std::vector<std::string> &rows, const std::string &fileId, Kernel &kernel)>> kernelParseMap;
    const std::string kernelDetailReg = R"((kernel_details|op_summary[_\d]*)\.csv$)";

    static void InitkernelParseMap();

    static std::map<std::string, std::vector<std::string>> GetKernelFiles(const std::vector<std::string>& paths);

    static bool IsFileValid(const std::vector<std::string>& filePathList, const std::string &fileId,
                            const std::string& statusId, std::string &message);
    static void SetParseCallBack(const std::string& token);
    static void ParseEndCallBack(const std::string& fileId, bool result, const std::string &msg);
    static void ParseCallBack(const std::string &token, const std::string& fileId, bool result, const std::string &msg);

    static void PreParseTask(const std::vector<std::string>& filePathList, const std::string &fileId);
    static bool ParseTask(const std::vector<std::string>& filePathList, const std::string &fileId,
                          std::string &message);

    static void ParseMsprofKernel(const std::map<std::string, size_t> &dataMap, const std::vector<std::string> &rows,
                                  const std::string &fileId, Kernel &kernel);
    static void ParseAscendPyTorchprofKernel(const std::map<std::string, size_t> &dataMap,
                                             const std::vector<std::string> &rows, const std::string &fileId,
                                             Kernel &kernel);
    static void ParseMsprofKernelL0(const std::map<std::string, size_t> &dataMap,
                                    const std::vector<std::string> &rows, const std::string &fileId, Kernel &kernel);
    static void ParseAscendPyTorchprofKernelL0(const std::map<std::string, size_t> &dataMap,
                                               const std::vector<std::string> &rows, const std::string &fileId,
                                               Kernel &kernel);
    static void ParsePublicNotL0(const std::map<std::string, size_t> &dataMap, const std::vector<std::string> &rows,
                                 Kernel &kernel);
    static bool ParseKernelCsv(const std::string& filePath, const std::string &fileId, const std::string& statusId,
                        std::string &message, std::set<std::string>& devices);

    static bool InitParser(const std::vector<std::string>& filePathList, const std::string &fileId,
                           std::string &message);
    static void PostParseTask(const std::set<std::string> &devices, const std::string &fileId);
    static bool CheckHeaderField(const std::map<std::string, size_t>& dataMap);
};

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_KERNELPARSE_H
