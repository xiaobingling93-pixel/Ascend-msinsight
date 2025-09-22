/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_ABSTRACTPARSEUNIT_H
#define PROFILER_SERVER_ABSTRACTPARSEUNIT_H
#include <string>
#include "DbTraceDataBase.h"

namespace Dic::Module::FullDb {
struct ParseUnitParams {
    std::string dbId;
};

class AbstractParseUnit {
public:
    AbstractParseUnit() = default;
    virtual ~AbstractParseUnit() = default;
    bool Handle(const ParseUnitParams &params);

protected:
    // 获取解析单元名称
    virtual std::string GetUnitName() = 0;
    // 前置校验
    virtual bool PreCheck(const ParseUnitParams &params, const std::shared_ptr<DbTraceDataBase> &database,
                          std::string &error) = 0;
    // 处理解析流程
    virtual bool HandleParseProcess(const ParseUnitParams &params, const std::shared_ptr<DbTraceDataBase> &database,
                                    std::string &error) = 0;
    // 发送通知:默认实现 有特殊要求 请在子类进行覆盖
    virtual void SendNotify(const ParseUnitParams &params, bool parseRes,
                            const std::string &error);

private:
    bool Parse(const ParseUnitParams &params, std::string &error);
};
}
#endif // PROFILER_SERVER_ABSTRACTPARSEUNIT_H
