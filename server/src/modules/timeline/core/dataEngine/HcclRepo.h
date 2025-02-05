//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_HCCLREPO_H
#define PROFILER_SERVER_HCCLREPO_H
#include "TaskTable.h"
#include "CommucationTaskInfoTable.h"
#include "CommucationOpTable.h"
#include "TrackInfoManager.h"
#include "StringIdsTable.h"
#include "EnumHcclDataTypeTable.h"
#include "EnumHcclLinkTypeTable.h"
#include "EnumHcclTransportTypeTable.h"
#include "EnumHcclRdmaTypeTable.h"
#include "NpuInfoRepo.h"
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
    bool QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain);
    void SetTaskTable(std::unique_ptr<TaskTable>);
    void SetCommucationOpTable(std::unique_ptr<CommucationOpTable>);
    void SetNpuInfoRepo(std::unique_ptr<NpuInfoRepo> npuInfoRepoPtr);
    void SetCommucationTaskInfoTable(std::unique_ptr<CommucationTaskInfoTable>);

protected:
    std::unique_ptr<TaskTable> taskTable = std::make_unique<TaskTable>();
    std::unique_ptr<CommucationOpTable> commucationOpTable = std::make_unique<CommucationOpTable>();
    std::unique_ptr<NpuInfoRepo> npuInfoRepo = std::make_unique<NpuInfoRepo>();
    std::unique_ptr<CommucationTaskInfoTable> commucationTaskInfoTable = std::make_unique<CommucationTaskInfoTable>();
    std::unique_ptr<StringIdsTable> stringIdsTable = std::make_unique<StringIdsTable>();
    std::unique_ptr<EnumHcclDataTypeTable> enumHcclDataTypeTable = std::make_unique<EnumHcclDataTypeTable>();
    std::unique_ptr<EnumHcclTransportTypeTable> enumHcclTransportTypeTable =
        std::make_unique<EnumHcclTransportTypeTable>();
    std::unique_ptr<EnumHcclLinkTypeTable> enumHcclLinkTypeTable = std::make_unique<EnumHcclLinkTypeTable>();
    std::unique_ptr<EnumHcclRdmaTypeTable> enumHcclRdmaTypeTable = std::make_unique<EnumHcclRdmaTypeTable>();

private:
    const std::string groupSuffix = "group";

    std::vector<uint64_t> QueryGlobalTaskIdsByRank(const TrackInfo &trackInfo);

    std::vector<uint64_t> QueryOpIdsByGlabalTaskIds(const TrackInfo &trackInfo, const std::vector<uint64_t> &globalIds);

    void QueryGroupSliceByIds(const std::vector<uint64_t> &sliceIds, std::vector<CompeteSliceDomain> &competeSliceVec,
        const TrackInfo &trackInfo);

    void QueryPlaneSliceByIds(const std::vector<uint64_t> &sliceIds, std::vector<CompeteSliceDomain> &competeSliceVec,
        const TrackInfo &trackInfo);

    void QuerySimpleSliceFromGroupTrack(std::vector<SliceDomain> &sliceVec, const TrackInfo &trackInfo,
        const std::string &suffix);

    void QuerySimpleSliceFromPlaneTrack(std::vector<SliceDomain> &sliceVec, TrackInfo &trackInfo);

    bool QueryGroupSliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain,
        const TrackInfo &trackInfo);

    std::string QueryTransportName(const SliceQuery &sliceQuery, CommucationTaskInfoPO &targetTaskInfo);

    std::string QueryDataTypeName(const SliceQuery &sliceQuery, CommucationTaskInfoPO &targetTaskInfo);

    std::string QueryLinkTypeName(const SliceQuery &sliceQuery, CommucationTaskInfoPO &targetTaskInfo);

    std::string QueryRdmaTypeName(const SliceQuery &sliceQuery, CommucationTaskInfoPO &targetTaskInfo);

    void SetPlaneSliceArgs(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain,
        const TaskPO &targetPO);

    bool QueryPlaneSliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain);
};
}
#endif // PROFILER_SERVER_HCCLREPO_H
