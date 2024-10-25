/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <sqlite3.h>
#include <string>
namespace Dic::Global::PROFILER::MockUtil {
class DatabaseTestCaseMockUtil {
public:
    static void OpenDB(sqlite3 *&db)
    {
        // 在内存中创建SQLite数据库
        int rc = sqlite3_open(":memory:", &db);
        if (rc != SQLITE_OK) {
            std::cout << "create db failed" << std::endl;
            sqlite3_close(db);
        }
    }

    static void CreateTable(sqlite3 *&db, const std::string &sql)
    {
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            std::cout << "insert bd is failed" << std::endl;
            sqlite3_free(nullptr);
            sqlite3_close(db);
        }
    }

    static void OpenDBAndCreateTable(sqlite3 *&db, const std::string &sql)
    {
        OpenDB(db);
        CreateTable(db, sql);
    }

    static void InsertData(sqlite3 *&db, const std::string &sql)
    {
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            std::cout << "create table is failed" << std::endl;
            sqlite3_free(nullptr);
            sqlite3_close(db);
            return;
        }
    }
};
}