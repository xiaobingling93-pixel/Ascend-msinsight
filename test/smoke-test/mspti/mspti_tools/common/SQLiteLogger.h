#pragma once
#include <sqlite3.h>
#include <string>
#include "ActivityRegistry.h"

class SQLiteLogger {
public:
    // 获取单例实例
    static SQLiteLogger& instance() {
        static SQLiteLogger logger("activity_log");
        return logger;
    }

    template<typename T>
    void insertRecords(const std::vector<T> &records) {
        if (records.empty()) return;

        auto &info = ActivityTraits<T>::get();
        const auto &fields = info.fields;

        std::string sql = "INSERT INTO " + info.tableName + " (";
        for (size_t i = 0; i < fields.size(); ++i) {
            sql += fields[i].name;
            if (i < fields.size() - 1) sql += ",";
        }
        sql += ") VALUES (";
        for (size_t i = 0; i < fields.size(); ++i) {
            sql += "?";
            if (i < fields.size() - 1) sql += ",";
        }
        sql += ");";

        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Prepare failed: " + std::string(sqlite3_errmsg(db_)));
        }

        // 开始事务
        char *errMsg = nullptr;
        if (sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Begin transaction failed: " + std::string(errMsg));
        }

        for (const auto &record : records) {
            const char *base = reinterpret_cast<const char *>(&record);
            for (size_t i = 0; i < fields.size(); ++i) {
                const void *fieldPtr = base + fields[i].offset;
                switch (fields[i].type) {
                    case FieldType::INTEGER_32:
                        sqlite3_bind_int64(stmt, static_cast<int>(i + 1), *reinterpret_cast<const int32_t *>(fieldPtr));
                        break;
                    case FieldType::INTEGER:
                        sqlite3_bind_int64(stmt, static_cast<int>(i + 1), *reinterpret_cast<const int64_t *>(fieldPtr));
                        break;
                    case FieldType::REAL:
                        sqlite3_bind_double(stmt, static_cast<int>(i + 1), *reinterpret_cast<const double *>(fieldPtr));
                        break;
                    case FieldType::TEXT:
                        sqlite3_bind_text(stmt, static_cast<int>(i + 1),
                            *reinterpret_cast<const char * const *>(fieldPtr) ? *reinterpret_cast<const char * const *>(fieldPtr) : "",
                            -1, SQLITE_TRANSIENT);
                        break;
                }
            }

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
                throw std::runtime_error("Insert failed: " + std::string(sqlite3_errmsg(db_)));
            }

            sqlite3_reset(stmt);  // 重置语句以便下一次绑定
        }

        sqlite3_finalize(stmt);

        // 提交事务
        if (sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
            throw std::runtime_error("Commit failed: " + std::string(errMsg));
        }
    }

private:
    SQLiteLogger(const std::string &baseName);
    ~SQLiteLogger();
    void createAllTables();

private:
    sqlite3 *db_ = nullptr;
    std::string dbFile_;
};
