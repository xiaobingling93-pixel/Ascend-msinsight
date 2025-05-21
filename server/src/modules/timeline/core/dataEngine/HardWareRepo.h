//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//
#ifndef PROFILER_SERVER_HARDWAREREPO_H
#define PROFILER_SERVER_HARDWAREREPO_H
#include "TaskTable.h"
#include "ComputeTaskInfoTable.h"
#include "StringIdsTable.h"
#include "SliceRepoInterface.h"
namespace Dic::Module::Timeline {
using Stmt = std::unique_ptr<SqlitePreparedStatement>;
class HardWareRepo : public SliceRepoInterface {
public:
    void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
        std::vector<SliceDomain> &sliceVec) override;
    void QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds) override;
    uint64_t QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery) override;
    void QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
        std::vector<CompeteSliceDomain> &sliceVec) override;
    void QueryAllThreadInfo(const ThreadQuery &flowQuery,
        std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo) override;
    void QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
        std::vector<CompeteSliceDomain> &CompeteSliceVec) override;
    bool QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;
    bool QuerySliceDetailInfoByNameList(const SliceQueryByNameList &params,
                                        std::vector<CompeteSliceDomain> &res) override;
protected:
    std::unique_ptr<TaskTable> taskTable = std::make_unique<TaskTable>();
    std::unique_ptr<ComputeTaskInfoTable> computeTaskInfoTable = std::make_unique<ComputeTaskInfoTable>();
    std::unique_ptr<StringIdsTable> stringIdsTable = std::make_unique<StringIdsTable>();

    virtual Stmt CreatPreparedStatement(const std::string &sql, const SliceQuery &sliceQuery);
    std::string GetDbPath(const SliceQuery &sliceQuery);
    void QuerySliceArgs(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain, const TaskPO &targetTask);

    void
    QuerySliceShape(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain, const TaskPO &targetTask);
    bool QueryMemoryInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain,
                         const TaskPO &targetTask);
};
}
#endif // PROFILER_SERVER_HARDWAREREPO_H
