/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
 
#include "PythonGCTable.h"
namespace Dic::Module::Timeline {
void PythonGCTable::SetId(PythonGCPO &pythonGCPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    pythonGCPO.id = resultSet->GetUint64(MstxEventsColumn::ID);
}
 
void PythonGCTable::SetTimestamp(PythonGCPO &pythonGCPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    pythonGCPO.timestamp = resultSet->GetUint64(MstxEventsColumn::TIMESTAMP);
}
 
void PythonGCTable::SetEndTime(PythonGCPO &pythonGCPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    pythonGCPO.endTime = resultSet->GetUint64(MstxEventsColumn::ENDTIME);
}
}