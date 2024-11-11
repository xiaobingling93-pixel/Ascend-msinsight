/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_PYTORCHCALLCHAINS_H
#define PROFILER_SERVER_PYTORCHCALLCHAINS_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct PytorchCallchainsPO {
    uint64_t id = 0;
    uint64_t stack = 0;
    uint64_t stackDepth = 0;
};

class PytorchCallchainsTable : public Table<PytorchCallchainsPO> {
public:
    PytorchCallchainsTable() = default;
    ~PytorchCallchainsTable() override = default;

protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { PytorchCallchainsColumn::ID, SetId },
            { PytorchCallchainsColumn::STACK, SetStack },
            { PytorchCallchainsColumn::STACK_DEPTH, SetStackDepth },
        };
        return assignMap;
    }

    const std::string &GetTableName() override
    {
        static std::string tableName = "PYTORCH_CALLCHAINS";
        return tableName;
    }
    static void SetId(PytorchCallchainsPO &pytorchApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetStack(PytorchCallchainsPO &pytorchApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetStackDepth(PytorchCallchainsPO &pytorchApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
};
#endif // PROFILER_SERVER_PYTORCHCALLCHAINS_H
