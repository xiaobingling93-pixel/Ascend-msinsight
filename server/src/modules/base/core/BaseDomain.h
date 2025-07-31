/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_BASEDOMAIN_H
#define PROFILER_SERVER_BASEDOMAIN_H
#include <vector>
#include <string>
#include <map>
namespace Dic::Module {
struct ColumnAtt {
    std::string name;
    std::string type;
    std::string key;
};

struct TableDatas {
    std::vector<ColumnAtt> att;
    std::vector<std::map<std::string, std::string>> datas;
    uint64_t count = 0;
};
struct PageFilter {
    std::string col;
    std::string content;
};
struct PageQuery {
    uint64_t size = 0;
    uint64_t curPage = 0;
    std::string fileId;
    std::string orderBy;
    std::string order;
    std::string viewName;
    std::string start;
    std::string end;
    std::vector<PageFilter> pageFilters;
    uint64_t ComputeOffset() const
    {
        if (curPage == 0 || size == 0) {
            return 0;
        }
        if (curPage - 1 > UINT64_MAX / size) {
            return 0;
        }
        return (curPage - 1) * size;
    }
};
}  // namespace Dic::Module
#endif  // PROFILER_SERVER_BASEDOMAIN_H
