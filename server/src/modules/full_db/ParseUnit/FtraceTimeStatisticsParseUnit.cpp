/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "FtraceTimeStatisticsParseUnit.h"
#include "ConstantDefs.h"
#include "ParseUnitManager.h"

namespace Dic::Module::FullDb {

std::string FtraceTimeStatisticsParseUnit::GetUnitName()
{
    return Dic::FTRACE_TIME_STATISTICS_UNIT;
}

bool FtraceTimeStatisticsParseUnit::PreCheck(const ParseUnitParams &params,
                                              const std::shared_ptr<Timeline::TextTraceDatabase> &database,
                                              std::string &error)
{
    return true;
}

bool FtraceTimeStatisticsParseUnit::HandleParseProcess(const ParseUnitParams &params,
                                                        const std::shared_ptr<Timeline::TextTraceDatabase> &database,
                                                        std::string &error)
{
    return true;
}

ParseUnitRegistrar<FtraceTimeStatisticsParseUnit> unitRegFtraceTime(Dic::FTRACE_TIME_STATISTICS_UNIT);

}
