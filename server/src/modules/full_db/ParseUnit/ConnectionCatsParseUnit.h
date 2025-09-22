/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_CONNECTIONCATSPARSEUNIT_H
#define PROFILER_SERVER_CONNECTIONCATSPARSEUNIT_H
#include "AbstractParseUnit.h"
namespace Dic::Module::FullDb {
    class ConnectionCatsParseUnit : public AbstractParseUnit {
    protected:
        std::string GetUnitName() override;
        bool PreCheck(const ParseUnitParams &params, const std::shared_ptr<DbTraceDataBase> &database,
                      std::string &error) override;
        bool HandleParseProcess(const ParseUnitParams &params, const std::shared_ptr<DbTraceDataBase> &database,
                                std::string &error) override;
    };
}
#endif // PROFILER_SERVER_CONNECTIONCATSPARSEUNIT_H
