//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_HCCLREPO_H
#define PROFILER_SERVER_HCCLREPO_H
#include "TaskTable.h"
#include "CommucationTaskInfoTable.h"
#include "CommucationOpTable.h"
#include "TrackInfoManager.h"
#include "SliceRepoInterface.h"
namespace Dic::Module::Timeline {
class HcclRepo : public SliceRepoInterface {
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
    void SetTaskTable(std::unique_ptr<TaskTable>);
    void SetCommucationOpTable(std::unique_ptr<CommucationOpTable>);
    void SetCommucationTaskInfoTable(std::unique_ptr<CommucationTaskInfoTable>);

private:
    std::unique_ptr<TaskTable> taskTable = std::make_unique<TaskTable>();
    std::unique_ptr<CommucationOpTable> commucationOpTable = std::make_unique<CommucationOpTable>();
    std::unique_ptr<CommucationTaskInfoTable> commucationTaskInfoTable = std::make_unique<CommucationTaskInfoTable>();

    std::vector<uint64_t> QueryGlobalTaskIdsByRank(const TrackInfo &trackInfo);

    std::vector<uint64_t> QueryOpIdsByGlabalTaskIds(const TrackInfo &trackInfo, const std::vector<uint64_t> &globalIds);

    void QueryGroupSliceByIds(const std::vector<uint64_t> &sliceIds, std::vector<CompeteSliceDomain> &competeSliceVec,
        const TrackInfo &trackInfo);

    void QueryPlaneSliceByIds(const std::vector<uint64_t> &sliceIds, std::vector<CompeteSliceDomain> &competeSliceVec,
        const TrackInfo &trackInfo);

    void QuerySimpleSliceFromGroupTrack(std::vector<SliceDomain> &sliceVec, const TrackInfo &trackInfo,
        const std::string &suffix);

    void QuerySimpleSliceFromPlaneTrack(std::vector<SliceDomain> &sliceVec, TrackInfo &trackInfo);
};
}
#endif // PROFILER_SERVER_HCCLREPO_H
