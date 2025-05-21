/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IECONTEXT_H
#define PROFILER_SERVER_IECONTEXT_H
#include <mutex>
#include <string>
#include <map>
#include <IEConnectionPool.h>
namespace Dic::Module::IE {
class IEContext {
public:
    static IEContext& Instance();
    IEContext(const IEContext &) = delete;
    IEContext &operator = (const IEContext &) = delete;
    IEContext(IEContext &&) = delete;
    IEContext &operator = (IEContext &&) = delete;

    bool InitDataBase(const std::string &fileId, const std::string &dbPath);
    std::string ComputeFileIdByFolder(const std::string &folder);
    bool ExecSql(const std::string &fileId, const std::string &sql);
    std::shared_ptr<Database> GetDatabase(const std::string &fileId);
    /* *
     * 清理所有上下文
     */
    void Reset();

private:
    std::mutex mutex;
    std::map<std::string, std::unique_ptr<IEConnectionPool>> databaseMap;
    std::map<std::string, std::recursive_mutex> dbMutexMap;
    std::map<std::string, std::string> folderFileIdMap;
    IEContext() = default;
    ~IEContext() = default;
};
}
#endif // PROFILER_SERVER_IECONTEXT_H
