/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TABLE_H
#define PROFILER_SERVER_TABLE_H
#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <functional>
#include <unordered_map>
#include "SqlitePreparedStatement.h"
#include "SqliteResultSet.h"
#include "sqlite3.h"
#include "ServerLog.h"
namespace Dic::Module::Timeline {
using namespace Dic::Server;
enum class TableOrder {
    ASC,
    DESC,
};
template <typename T> class Table {
public:
    Table() noexcept
    {
        selectStr = "";
        conditionStr = "";
        sql = "";
        orderByStr = "";
    }
    Table &Select(std::string_view strView)
    {
        std::string str{strView};
        if (std::empty(selectStr)) {
            selectStr = "SELECT " + str;
        } else {
            selectStr += "," + str;
        }
        assignFuncs.emplace_back(GetAssignMap()[str]);
        return *this;
    }

    template <typename... Args> Table &Select(std::string_view strView, Args ... args)
    {
        std::string str{strView};
        if (std::empty(selectStr)) {
            selectStr = "SELECT " + str;
        } else {
            selectStr += " , " + str;
        }
        assignFuncs.emplace_back(GetAssignMap()[str]);
        Select(args...);
        return *this;
    }

    Table &Eq(std::string_view strView, std::variant<uint32_t, uint64_t, std::string> value)
    {
        std::string str{strView};
        conditionStr += " AND " + str + " = ? ";
        values.emplace_back(value);
        return *this;
    }

    Table &NotEq(const std::string &str, std::variant<uint32_t, uint64_t, std::string> value)
    {
        conditionStr += " AND " + str + " != ? ";
        values.emplace_back(value);
        return *this;
    }

    Table &Less(const std::string &str, std::variant<uint32_t, uint64_t, std::string> value)
    {
        conditionStr += " AND " + str + " < ? ";
        values.emplace_back(value);
        return *this;
    }

    Table &LessEq(std::string_view strView, std::variant<uint32_t, uint64_t, std::string> value)
    {
        std::string str{strView};
        conditionStr += " AND " + str + " <= ? ";
        values.emplace_back(value);
        return *this;
    }

    Table &Greater(std::string_view strView, std::variant<uint32_t, uint64_t, std::string> value)
    {
        std::string str{strView};
        conditionStr += " AND " + str + " > ? ";
        values.emplace_back(value);
        return *this;
    }

    Table &GreaterEq(const std::string &str, std::variant<uint32_t, uint64_t, std::string> value)
    {
        conditionStr += " AND " + str + " >= ? ";
        values.emplace_back(value);
        return *this;
    }

    Table &OrderBy(std::string_view columnName, TableOrder order)
    {
        std::string strColumnName{columnName};
        if (std::empty(orderByStr)) {
            orderByStr = " ORDER BY " + strColumnName;
        } else {
            orderByStr += " , " + strColumnName;
        }
        if (order == TableOrder::DESC) {
            orderByStr += " DESC ";
        } else {
            orderByStr += " ASC ";
        }
        return *this;
    }

    void ExcuteQuery(sqlite3 *db, std::vector<T> &result)
    {
        sql = selectStr + " FROM " + GetTableName() + " WHERE 1 = 1 " + conditionStr + orderByStr;
        auto stmt = CreatPreparedStatement(db);
        if (stmt == nullptr) {
            ServerLog::Error(GetTableName() + " Failed to prepare sql.");
            return;
        }
        for (const auto &item : values) {
            // 访问 variant 中存储的值
            if (std::holds_alternative<uint32_t>(item)) {
                stmt->BindParams(std::get<uint32_t>(item));
            } else if (std::holds_alternative<uint64_t>(item)) {
                stmt->BindParams(std::get<uint64_t>(item));
            } else if (std::holds_alternative<std::string>(item)) {
                stmt->BindParams(std::get<std::string>(item));
            }
        }
        auto resultSet = stmt->ExecuteQuery();
        if (resultSet == nullptr) {
            ServerLog::Error(GetTableName() + " Failed to get result set.", stmt->GetErrorMessage());
            return;
        }
        while (resultSet->Next()) {
            T t;
            for (const auto &item : assignFuncs) {
                item(t, resultSet);
            }
            result.emplace_back(t);
        }
        ClearThreadLocal();
    }

    uint64_t Count(sqlite3 *db)
    {
        uint64_t count = 0;
        sql = "SELECT COUNT(*) AS count FROM " + GetTableName() + " WHERE 1 = 1 " + conditionStr;
        auto stmt = CreatPreparedStatement(db);
        if (stmt == nullptr) {
            ServerLog::Error(GetTableName() + " Failed to prepare sql.");
            return count;
        }
        for (const auto &item : values) {
            // 访问 variant 中存储的值
            if (std::holds_alternative<uint32_t>(item)) {
                stmt->BindParams(std::get<uint32_t>(item));
            } else if (std::holds_alternative<uint64_t>(item)) {
                stmt->BindParams(std::get<uint64_t>(item));
            } else if (std::holds_alternative<std::string>(item)) {
                stmt->BindParams(std::get<std::string>(item));
            }
        }
        auto resultSet = stmt->ExecuteQuery();
        if (resultSet == nullptr) {
            ServerLog::Error(GetTableName() + " Failed to get result set.", stmt->GetErrorMessage());
            return count;
        }
        if (resultSet->Next()) {
            count = resultSet->GetUint64("count");
        }
        ClearThreadLocal();
        return count;
    }

protected:
    std::string selectStr;
    std::string conditionStr;
    std::string orderByStr;
    std::string sql;
    std::vector<std::variant<uint32_t, uint64_t, std::string>> values;
    using assign = std::function<void(T &, const std::unique_ptr<SqliteResultSet> &)>;
    std::vector<assign> assignFuncs;
    std::unique_ptr<SqlitePreparedStatement> CreatPreparedStatement(sqlite3 *db)
    {
        if (sql.empty()) {
            ServerLog::Error(GetTableName() + " Failed prepare sql. Database is closed or sql is empty.");
            return nullptr;
        }
        auto stmt = std::make_unique<SqlitePreparedStatement>(db);
        if (!stmt->Prepare(sql)) {
            ServerLog::Error(GetTableName() + " Failed prepare sql. ", stmt->GetErrorMessage());
            return nullptr;
        }
        return stmt;
    }

    /* *
     * 每次调用完成需要重置threadlocal变量
     */
    void ClearThreadLocal()
    {
        assignFuncs.clear();
        values.clear();
        sql.clear();
        conditionStr.clear();
        selectStr.clear();
        orderByStr.clear();
    }

    virtual std::unordered_map<std::string_view, assign> &GetAssignMap() = 0;
    virtual std::string &GetTableName() = 0;
};
}


#endif // PROFILER_SERVER_TABLE_H
