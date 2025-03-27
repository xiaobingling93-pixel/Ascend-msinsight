/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Protocol Requests declaration
 */

#ifndef PROFILER_SERVER_PROTOCOLPARAMUTIL_H
#define PROFILER_SERVER_PROTOCOLPARAMUTIL_H

#include <string>
#include "ProtocolMessage.h"

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

struct ThreadDetailParams {
    std::string rankId;
    std::string pid;
    std::string tid;
    std::string id;
    std::string metaType;
    uint64_t startTime = 0;
    uint32_t depth = 0;
};

struct SetCardAliasParams {
    std::string rankId;
    std::string cardAlias;
    bool CheckParams(std::string alias, std::string &errMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValidEmptyAllowed(alias, paramError)) {
            errMsg = "[Timeline]Failed to check card alias, " + paramError;
            return false;
        }
        return true;
    }
};
}
#endif // PROFILER_SERVER_PROTOCOLPARAMUTIL_H