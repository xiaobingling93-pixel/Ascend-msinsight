/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_TIMELINEBODYSTRUCT_H
#define PROFILER_SERVER_TIMELINEBODYSTRUCT_H
#include <string>
#include <map>
#include <vector>
namespace Dic::Protocol {
struct TableColumn {
    std::string name;
    std::string type;
    std::string key;
};

struct TableDataDatail {
    std::vector<TableColumn> columnAttr;
    std::vector<std::map<std::string, std::string>> columnData;
    uint64_t totalNum = 0;
};

struct TableDataListBody {
    std::vector<std::string> layers;
};

struct SearchAllSlices {
    std::string name;
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    std::string id;
    std::string pid;
    std::string tid;
    uint64_t depth = 0;
    std::string rankId;
    std::string deviceId;
    std::string fileId;
};

struct SearchAllSlicesBody {
    std::vector<SearchAllSlices> searchAllSlices;
    std::string dbPath;
    uint64_t count{};
    uint64_t pageSize{};
    uint64_t currentPage{};
};

struct SameOperatorsDetails {
    uint64_t timestamp{};
    uint64_t duration{};
    // id、depth用于支持选中列表;
    std::string id;
    // name用于支持overall metric more details列表
    std::string name;
    uint64_t depth{};
    std::string tid;
};

struct UnitThreadsOperatorsBody {
    std::vector<SameOperatorsDetails> sameOperatorsDetails;
    std::string rankId;
    uint64_t count{};
    uint64_t pageSize{};
    uint64_t currentPage{};
};
}  // namespace Dic::Protocol
#endif  // PROFILER_SERVER_TIMELINEBODYSTRUCT_H
