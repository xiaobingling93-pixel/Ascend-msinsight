#include "SQLiteLogger.h"
#include "mspti.h"
#include <unistd.h>
#include <stdexcept>

SQLiteLogger::SQLiteLogger(const std::string &baseName) {
    pid_t pid = getpid();
    dbFile_ = baseName + "_" + std::to_string(pid) + ".db";
    if (sqlite3_open(dbFile_.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db_)));
    }
    createAllTables();
}

SQLiteLogger::~SQLiteLogger() {
    if (db_) sqlite3_close(db_);
}

void SQLiteLogger::createAllTables() {
    for (auto &t : getRegistry()) {
        std::string sql = "CREATE TABLE IF NOT EXISTS " + t.tableName + " (id INTEGER PRIMARY KEY AUTOINCREMENT";
        for (auto &f : t.fields) {
            sql += ", " + f.name + " " + sqlType(f.type);
        }
        sql += ");";

        char *errMsg = nullptr;
        if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string err = errMsg ? errMsg : "Unknown error";
            sqlite3_free(errMsg);
            throw std::runtime_error("Create table failed: " + err);
        }
    }
}

inline const std::vector<FieldDef> msptiActivityKernelFields = {
    {"kind", FieldType::INTEGER_32, offsetof(msptiActivityKernel, kind)},
    {"start", FieldType::INTEGER, offsetof(msptiActivityKernel, start)},
    {"end", FieldType::INTEGER, offsetof(msptiActivityKernel, end)},
    {"deviceId", FieldType::INTEGER_32, offsetof(msptiActivityKernel, ds.deviceId)},
    {"streamId", FieldType::INTEGER_32, offsetof(msptiActivityKernel, ds.streamId)},
    {"correlationId", FieldType::INTEGER, offsetof(msptiActivityKernel, correlationId)},
    {"type", FieldType::TEXT, offsetof(msptiActivityKernel, type)},
    {"name", FieldType::TEXT, offsetof(msptiActivityKernel, name)}
};
REGISTER_ACTIVITY_TYPE(msptiActivityKernel, "kernel", msptiActivityKernelFields);

inline const std::vector<FieldDef> msptiActivityHcclFields = {
    {"kind", FieldType::INTEGER_32, offsetof(msptiActivityHccl, kind)},
    {"start", FieldType::INTEGER, offsetof(msptiActivityHccl, start)},
    {"end", FieldType::INTEGER, offsetof(msptiActivityHccl, end)},
    {"deviceId", FieldType::INTEGER_32, offsetof(msptiActivityHccl, ds.deviceId)},
    {"streamId", FieldType::INTEGER_32, offsetof(msptiActivityHccl, ds.streamId)},
    {"bandWidth", FieldType::REAL, offsetof(msptiActivityHccl, bandWidth)},
    {"name", FieldType::TEXT, offsetof(msptiActivityHccl, name)},
    {"commName", FieldType::TEXT, offsetof(msptiActivityHccl, commName)}
};
REGISTER_ACTIVITY_TYPE(msptiActivityHccl, "hccl", msptiActivityHcclFields);

inline const std::vector<FieldDef> msptiActivityApiFields = {
    {"kind", FieldType::INTEGER_32, offsetof(msptiActivityApi, kind)},
    {"start", FieldType::INTEGER, offsetof(msptiActivityApi, start)},
    {"end", FieldType::INTEGER, offsetof(msptiActivityApi, end)},
    {"processId", FieldType::INTEGER_32, offsetof(msptiActivityApi, pt.processId)},
    {"threadId", FieldType::INTEGER_32, offsetof(msptiActivityApi, pt.threadId)},
    {"correlationId", FieldType::INTEGER, offsetof(msptiActivityApi, correlationId)},
    {"name", FieldType::TEXT, offsetof(msptiActivityApi, name)}
};
REGISTER_ACTIVITY_TYPE(msptiActivityApi, "api", msptiActivityApiFields);

inline const std::vector<FieldDef> msptiActivityCommFields = {
    {"kind", FieldType::INTEGER_32, offsetof(msptiActivityCommunication, kind)},
    {"dataType", FieldType::INTEGER_32, offsetof(msptiActivityCommunication, dataType)},
    {"count", FieldType::INTEGER, offsetof(msptiActivityCommunication, count)},
    {"deviceId", FieldType::INTEGER_32, offsetof(msptiActivityCommunication, ds.deviceId)},
    {"streamId", FieldType::INTEGER_32, offsetof(msptiActivityCommunication, ds.streamId)},
    {"start", FieldType::INTEGER, offsetof(msptiActivityCommunication, start)},
    {"end", FieldType::INTEGER, offsetof(msptiActivityCommunication, end)},
    {"algType", FieldType::TEXT, offsetof(msptiActivityCommunication, algType)},
    {"name", FieldType::TEXT, offsetof(msptiActivityCommunication, name)},
    {"commName", FieldType::TEXT, offsetof(msptiActivityCommunication, commName)},
    {"correlationId", FieldType::INTEGER, offsetof(msptiActivityCommunication, correlationId)}
};
REGISTER_ACTIVITY_TYPE(msptiActivityCommunication, "communication", msptiActivityCommFields);

inline const std::vector<FieldDef> msptiActivityMarkFields = {
    {"kind", FieldType::INTEGER_32, offsetof(msptiActivityMarker, kind)},
    {"flag", FieldType::INTEGER_32, offsetof(msptiActivityMarker, flag)},
    {"sourceKind", FieldType::INTEGER_32, offsetof(msptiActivityMarker, sourceKind)},
    {"timestamp", FieldType::INTEGER, offsetof(msptiActivityMarker, timestamp)},
    {"markId", FieldType::INTEGER, offsetof(msptiActivityMarker, id)},
    {"deviceId", FieldType::INTEGER_32, offsetof(msptiActivityMarker, objectId.ds.deviceId)},
    {"streamId", FieldType::INTEGER_32, offsetof(msptiActivityMarker, objectId.ds.streamId)},
    {"name", FieldType::TEXT, offsetof(msptiActivityMarker, name)},
    {"domain", FieldType::TEXT, offsetof(msptiActivityMarker, domain)}
};
REGISTER_ACTIVITY_TYPE(msptiActivityMarker, "mark", msptiActivityMarkFields);