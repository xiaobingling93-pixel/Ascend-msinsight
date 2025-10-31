/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_CURVEREPO_H
#define PROFILER_SERVER_CURVEREPO_H
#include "ServitizationContext.h"
#include "BaseDomain.h"
namespace Dic::Module::IE {
class CurveRepo {
public:
    virtual std::vector<std::string> QueryAllViews(const std::string &fileId);
    virtual std::vector<ColumnAtt> QueryTableInfoByName(const std::string &fileId, const std::string &tableName);
    virtual std::string QueryTableNameDesc(const std::string &fileId, const std::string &tableName, bool isZh);
    virtual std::vector<std::map<std::string, std::string>> QueryDataByColumn(const std::string &fileId,
        const std::string &tableName, const std::vector<ColumnAtt> &columns);
    virtual std::vector<std::map<std::string, std::string>> QueryDataByColumnPage(const PageQuery &query,
        const std::vector<ColumnAtt> &columns);
    virtual uint64_t QueryCountByTableName(const PageQuery &query, const std::string& abscissa = "");
protected:
    std::shared_ptr<ServitizationContext> context = std::make_shared<ServitizationContext>();

    std::vector<std::string> ComputeColNames(const PageQuery &query, const std::vector<ColumnAtt> &columns) const;

    std::string &AppendConditionSql(const PageQuery &query, std::string &sql) const;
};
}
#endif // PROFILER_SERVER_CURVEREPO_H
