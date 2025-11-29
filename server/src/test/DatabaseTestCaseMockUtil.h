/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_DATABASETESTCASEMOCKUTIL_H
#define PROFILER_SERVER_DATABASETESTCASEMOCKUTIL_H

#include <iostream>
#include <sqlite3.h>
#include <string>
#include "DatabaseTestConst.h"
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
            std::cout << "Create table is failed" << std::endl;
            sqlite3_close(db);
        }
    }

    static void CreateTablesFromList(sqlite3 *&db, const std::vector<TableName> &list)
    {
        for (const auto &item : list) {
            if (CREATE_TABLE_SQL_MAP.find(item) != CREATE_TABLE_SQL_MAP.end()) {
                CreateTable(db, CREATE_TABLE_SQL_MAP.at(item));
            }
        }
    }

    static void ExecuteSql(sqlite3 *&db, const std::string &sql)
    {
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            std::cout << "Execute sql is failed" << std::endl;
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
            std::cout << "Insert data is failed" << std::endl;
            sqlite3_free(nullptr);
            sqlite3_close(db);
            return;
        }
    }
};
}

#endif // PROFILER_SERVER_DATABASETESTCASEMOCKUTIL_H
