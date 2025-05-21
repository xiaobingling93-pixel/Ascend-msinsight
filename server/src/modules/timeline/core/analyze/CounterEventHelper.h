/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_COUNTEREVENTHELPER_H
#define PROFILER_SERVER_COUNTEREVENTHELPER_H
#include <string>
#include <vector>
#include "FullDbEnumUtil.h"
#include "TraceDatabaseDef.h"
namespace Dic::Module::Timeline {
// progressName 进程名
// tableName db文件的表名
// valueName 作为柱状图值展现的列名称
// threadNameForamt 泳道线程名称，为格式字符串，{}外是固定内容，{}内的内容会被对应表中字段替换
// 比如"CPU {cpuId}"表示列名固定字段为"CPU "，看tableName表中有多少个不同的cpuId值，假设有0 1两个值，泳道线程名称就会返回CPU 0和CPU 1两个泳道
// type 前端展示需要的信息
struct CounterEventConfig {
    std::string progressName;
    std::string tableName;
    std::string valueName;
    std::string threadNameFormat;
    std::string type;
};

class CounterEventHelper {
public:
    CounterEventHelper() = default;
    ~CounterEventHelper() = default;
    void RegisterHostMap();
    std::string GenerateMetaDataSQL(PROCESS_TYPE type);
    std::string GenerateCounterSQL(PROCESS_TYPE type);
    std::map<PROCESS_TYPE, CounterEventConfig> hostCounterEventMap;
protected:
    std::string SubstituteThreadNameFormat(const std::string &format);
};
}
#endif // PROFILER_SERVER_COUNTEREVENTHELPER_H
