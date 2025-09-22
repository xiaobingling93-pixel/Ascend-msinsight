/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "OverlapAnalysisParseUnit.h"
#include "ParseUnitManager.h"
#include "ConstantDefs.h"
#include "TableDefs.h"

namespace Dic::Module::FullDb {
    std::string OverlapAnalysisParseUnit::GetUnitName()
    {
        return OVERLAP_ANALYSIS_UNIT;
    }

    bool OverlapAnalysisParseUnit::PreCheck(const ParseUnitParams &params,
                                            const std::shared_ptr<DbTraceDataBase> &database, std::string &error)
    {
        if (!database->CheckTableExist(TABLE_OVERLAP_ANALYSIS)) {
            error = "Generate overlap analysis:Table OVERLAP_ANALYSIS is not exist.";
            return false;
        }
        return true;
    }

    bool OverlapAnalysisParseUnit::HandleParseProcess(const ParseUnitParams &params,
                                                      const std::shared_ptr<DbTraceDataBase> &database, std::string &error)
    {
        if (!database->InitStmt()) {
            error = "Fail to init stmt when generate overlap analysis.";
            return false;
        }
        return database->GenerateOverlapAnalysis();
    }

    ParseUnitRegistrar<OverlapAnalysisParseUnit> unitRegOverlap(OVERLAP_ANALYSIS_UNIT);
}
