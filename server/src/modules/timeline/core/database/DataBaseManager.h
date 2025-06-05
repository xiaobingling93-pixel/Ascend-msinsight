/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DATABASEMANAGER_H
#define PROFILER_SERVER_DATABASEMANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <unordered_set>
#include "ConnectionPool.h"
#include "TextTraceDatabase.h"
#include "DbTraceDataBase.h"
#include "TextClusterDatabase.h"
#include "VirtualMemoryDataBase.h"
#include "LeaksMemoryDatabase.h"
#include "KernelParse.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Module::FullDb;
enum class DatabaseType {
    TRACE,
    SUMMARY,
    MEMORY,
    LEAKS
};
enum class DataType {
    TEXT,
    DB
};
enum class FileType {
    MS_PROF,
    PYTORCH,
    LEAKS
};
class DataBaseManager {
public:
    static DataBaseManager &Instance();
    DataBaseManager(const DataBaseManager &) = delete;
    DataBaseManager &operator=(const DataBaseManager &) = delete;
    DataBaseManager(DataBaseManager &&) = delete;
    DataBaseManager &operator=(DataBaseManager &&) = delete;

    bool CreatConnectionPool(const std::string &rankId, const std::string &dbPath);
    std::shared_ptr<VirtualTraceDatabase> GetTraceDatabaseByRankId(const std::string &rankId);
    std::shared_ptr<VirtualTraceDatabase> GetTraceDatabaseByFileId(const std::string& fileId);

    std::vector<ConnectionPool *> GetAllTraceDatabase();
    std::vector<std::string> GetAllRankId();
    void Clear();
    void Clear(DatabaseType type);
    void EraseClusterDb(const std::string &uniqueKey);
    void ClearClusterDb();
    void ReleaseDatabaseByRankId(const std::string &rankId);
    void ReleaseDatabaseByFileId(const std::string& fileId);
    bool HasRankId(DatabaseType type, const std::string &rankId);
    std::shared_ptr<VirtualClusterDatabase> CreateClusterDatabase(const std::string &uniqueKey, DataType type);
    std::shared_ptr<VirtualClusterDatabase> GetClusterDatabase(const std::string &uniqueKey);
    std::vector<std::shared_ptr<VirtualClusterDatabase>> GetAllClusterDatabase();

    std::shared_ptr<Memory::VirtualMemoryDataBase> CreateMemoryDataBase(const std::string &rankId,
                                                                        const std::string &dbPath);
    std::shared_ptr<Memory::VirtualMemoryDataBase> GetMemoryDatabaseByRankId(const std::string &rankId);
    std::shared_ptr<Memory::VirtualMemoryDataBase> GetMemoryDatabaseByFileId(const std::string& fileId);

    std::vector<Memory::VirtualMemoryDataBase *> GetAllMemoryDatabase();

    std::shared_ptr<FullDb::LeaksMemoryDatabase> GetLeaksMemoryDatabase(const std::string &fileId);
    std::vector<FullDb::LeaksMemoryDatabase *> GetAllLeaksMemoryDatabase();
    std::shared_ptr<Summary::VirtualSummaryDataBase> GetSummaryDatabaseByRankId(const std::string &rankId);
    std::shared_ptr<Summary::VirtualSummaryDataBase> GetSummaryDataBaseByFileId(const std::string &fileId);
    std::shared_ptr<Summary::VirtualSummaryDataBase> CreateSummaryDatabase(const std::string &rankId,
                                                                           const std::string &dbPath);
    std::vector<Summary::VirtualSummaryDataBase *> GetAllSummaryDatabase();

    std::string GetDbPathByRankId(const std::string &rankId);
    std::shared_ptr<VirtualTraceDatabase> GetTraceDatabaseWithOutHost(const std::string &rankId);
    DataType GetDataType();
    void SetDataType(DataType type);
    FileType GetFileType();
    FileType GetFileTypeByRankId(const std::string &rankId);
    void SetFileType(FileType type);
    void SetBaselineDataType(DataType type);
    void SetBaselineFileType(FileType type);
    bool ResetBaseline();
    void SetDbPathMapping(const std::string& rankId, const std::string& dbPath, const std::string& hostId);
    bool IsContainDatabasePath(const std::string& databasePath);
    std::string GetDeviceIdFromRankId(const std::string& rankId, const std::string& module);
    inline std::vector<std::string> GetDbPathByHost(const std::string& id)
    {
        if (host2DbPath.find(id) != host2DbPath.end()) {
            return host2DbPath[id];
        }
        return {};
    }
    std::string GetRankIdByFileId(const std::string &fileId);

    std::string GetAnyTraceDatabaseId();

    std::string GetFileIdByRankId(const std::string& rankId) const;
private:
    /**
     * @brief 设置rankId到fileId的映射
     * @param rankId
     * @param fileId
     * @param isBaseLine
     */
    void SetRankIdFileIdMapping(const std::string &rankId, const std::string &fileId, bool isBaseLine);
    
    using RankId = std::string;
    using FileId = std::string;
    using DbPath = FileId;
    using HostId = std::string;
    using ClusterPath = std::string;
    DataBaseManager() = default;
    ~DataBaseManager() = default;
    std::recursive_mutex mutex;
    DataType dataType = DataType::TEXT;
    DataType baselineType = DataType::TEXT;
    FileType fileType = FileType::PYTORCH;

    std::map<std::string, std::recursive_mutex> dbMutexMap;
    std::map<RankId, DbPath> dbFilePathMap;
    std::map<RankId, FileId> rankId2FileIdMap;
    std::map<FileId, RankId> fileIdToRankIdMap;
    std::map<HostId, std::vector<DbPath>> host2DbPath;
    std::unordered_set<std::string> databasePathSet;
    std::map<FileId, std::shared_ptr<ConnectionPool>> traceDatabaseMap;
    std::map<ClusterPath, std::shared_ptr<VirtualClusterDatabase>> clusterDatabaseMap;
    std::map<RankId, std::shared_ptr<Memory::VirtualMemoryDataBase>> memoryDatabaseMap;
    std::map<FileId, std::shared_ptr<FullDb::LeaksMemoryDatabase>> leaksMemoryDatabaseMap;
    std::map<RankId, std::shared_ptr<Summary::VirtualSummaryDataBase>> summaryDatabaseMap;

    FileType baselineFileType = FileType::PYTORCH;
    std::map<RankId, std::shared_ptr<Memory::VirtualMemoryDataBase>> memoryBaselineDatabaseMap;
    std::map<RankId, std::shared_ptr<Summary::VirtualSummaryDataBase>> summaryBaselineDatabaseMap;

    std::recursive_mutex &GetDbMutex(const std::string &fileId);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_DATABASEMANAGER_H
