/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVICE_DETAILSSERVICE_H
#define PROFILER_SERVICE_DETAILSSERVICE_H
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Protocol;

class DetailsService {
public:
static bool QueryDetailsLoadInfo(const SourceDetailsLoadInfoRequest &request, DetailsLoadInfoResponse &response);
static bool QueryMemoryGraph(const DetailsMemoryGraphRequest &request, DetailsMemoryGraphResponse &response);
static bool QueryMemoryTable(const DetailsMemoryTableRequest &request, DetailsMemoryTableResponse &response);
static bool QueryCoreLoadAnalysisGraph(const DetailsInterCoreLoadGraphRequest &request,
                                       DetailsInterCoreLoadGraphResponse &response);

private:
const static inline std::string underline = "_";
static SubBlockData MergeSubBlockData(const SubBlockData &compare, const SubBlockData& baseline);
static std::string GetSubBlockDataKey(const SubBlockUnitData &data);
static std::vector<MemoryGraph> MergeMemoryGraph(const std::vector<MemoryGraph> &compare,
    const std::vector<MemoryGraph> &baseline);
static CompareData<L2Cache> MergeL2Cache(const L2Cache &compare, const L2Cache &baseline);
static CompareData<UtilizationRate> MergeUtilizationRate(const UtilizationRate &compare,
    const UtilizationRate &baseline);
static std::vector<CompareData<MemoryUnit>> MergeMemoryUnit(const std::vector<CompareData<MemoryUnit>> &compare,
    const std::vector<CompareData<MemoryUnit>> &baseline);
static std::vector<MemoryTable> MergeMemoryTables(const std::vector<MemoryTable> &compare,
    const std::vector<MemoryTable> &baseline);
static std::vector<TableDetail<CompareData<TableRow>>> MergeCompareTableList(
    const std::vector<TableDetail<CompareData<TableRow>>> &compare,
    const std::vector<TableDetail<CompareData<TableRow>>> &baseline);
static std::vector<CompareData<TableRow>> MergeCompareRows(const std::vector<CompareData<TableRow>> &compare,
    const std::vector<CompareData<TableRow>> &baseline);
static std::vector<DetailsInterCoreLoadOpDetail> MergeCoreLoadOpDetail(
    const std::vector<DetailsInterCoreLoadOpDetail> &compare,
    const std::vector<DetailsInterCoreLoadOpDetail> &baseline);
static std::vector<DetailsInterCoreLoadSubCoreDetail> MergeCoreDetail(
    const std::vector<DetailsInterCoreLoadSubCoreDetail> &compare,
    const std::vector<DetailsInterCoreLoadSubCoreDetail> &baseline);
};
}
}
}
#endif // PROFILER_SERVICE_DETAILSSERVICE_H
