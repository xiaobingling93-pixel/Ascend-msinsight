/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Protocol Requests declaration
 */

#ifndef PROFILER_SERVER_PROTOCOLPARAMUTIL_H
#define PROFILER_SERVER_PROTOCOLPARAMUTIL_H

#include <string>

namespace Dic::Protocol {
struct OrderParam {
    std::string orderBy;
    std::string orderType;
    std::string NormalizeOrderType(const std::string& type) const
    {
        if (type == "ascend" || type == "Ascend") {
            return "ASC";
        } else {
            return "DESC";
        }
    }

    bool CheckOrderByInjection() const
    {
        return false;
    }

    std::string GenerateSql() const
    {
        std::string sql = " ";
        if (!orderBy.empty() && !orderType.empty()) {
            sql = " ORDER BY " + orderBy + " " + orderType + " ";
        }
        return sql;
    }
};

struct SearchParam {
    std::string condition;
    std::string type;
};

struct PageParam {
    uint32_t current{};
    uint32_t pageSize{};
    uint64_t total{};
    bool Check(std::string &errMsg) const
    {
        if (pageSize == 0) {
            errMsg = "Failed to check page parameter. Page size cannot be zero.";
            return false;
        }
        return true;
    }
};
}
#endif // PROFILER_SERVER_PROTOCOLPARAMUTIL_H