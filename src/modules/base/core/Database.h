/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_CORE_DATABASE_BASE_H
#define DATA_INSIGHT_CORE_MODULE_CORE_DATABASE_BASE_H

#include <vector>
#include <string>
#include <mutex>
#include "sqlite3.h"
#include "SqlitePreparedStatement.h"

namespace Dic {
namespace Module {
class Database {
public:
    Database() = default;
    virtual ~Database();
    virtual bool OpenDb(const std::string &dbPath, bool clearAllTable);
    virtual bool AttachDb(const std::string &dbPath);
    virtual bool IsOpen() const;
    void CloseDb();
    virtual bool StartTransaction();
    virtual bool EndTransaction();
    virtual std::string GetDbPath();
    virtual bool GetTableList(std::vector<std::string> &tableList) const;
    virtual std::unique_ptr<SqlitePreparedStatement> CreatPreparedStatement(const std::string &sql);

protected:
    bool ExecSql(const std::string &sql);
    static std::string CheckSqlString(const std::string &src);
    static std::string sqlite3_column_string(sqlite3_stmt *stmt, int iCol);
    std::string GetLastError();
    sqlite3 *db = nullptr;
    bool isOpen = false;
    std::string path;
    const int bindStartIndex = 1;
    const int resultStartIndex = 0;

private:
    bool DropAllTable();
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_CORE_DATABASE_BASE_H
