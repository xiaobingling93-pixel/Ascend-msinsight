/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef PROFILER_SERVER_STATISTICSMODULEAPI_H
#define PROFILER_SERVER_STATISTICSMODULEAPI_H
#include "ServitizationContext.h"
namespace Dic::Module::IE {
struct TaskInfo {
    std::string fileId;
    std::string filePath;
};
class ServitizationOpenApi {
public:
    ServitizationOpenApi() = default;
    ~ServitizationOpenApi() = default;
    virtual bool Parse(const std::unordered_map<std::string, std::string>& inputs);
    /**
     * 广度优先找到所有有效的IE文件
     * @param path
     * @return
     */
    virtual std::vector<TaskInfo> ComputeTaskInfo(const std::string& path);
    virtual bool CreateCurve(const std::string &fileId, const std::string &curve);
    virtual void Reset();

protected:
    std::shared_ptr<ServitizationContext> context = std::make_shared<ServitizationContext>();

private:
    const std::string IEFileName = "profiler.db";
    const std::string MS_SERVICE_PARSED_NAME = "ms_service_parsed.db";
    bool ValidIEFile(const std::string& path);
    void ParseSingleFile(const std::string& filePath, const std::string& fileId);
    bool ParseDir(const std::string& filePath, const std::string& fileId);

    void AttachDb(const std::string &dir, std::vector<std::string> &distributeFiles,
                  std::shared_ptr<Database> &database);
    void AttachSingleDb(const std::string &dir, const std::string &distributeFile,
                                              std::shared_ptr<Database> &database);
};
}  // namespace Dic::Module::IE
#endif  // PROFILER_SERVER_STATISTICSMODULEAPI_H
