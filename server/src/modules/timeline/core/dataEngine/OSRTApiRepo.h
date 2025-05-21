//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
//

#ifndef PROFILER_SERVER_OSRTAPI_H
#define PROFILER_SERVER_OSRTAPI_H
#include "SliceRepoInterface.h"
#include "VirtualTraceDatabase.h"
namespace Dic::Module::Timeline {
class OSRTApiRepo : public SliceRepoInterface {
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
                                std::vector<CompeteSliceDomain> &competeSliceVec) override;
    bool QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;
protected:
    void QuerySimpleSliceWithOutNameByTrackIdExecuteSQL(std::shared_ptr<VirtualTraceDatabase> database,
                                                        std::string &processId, std::vector<SliceDomain> &sliceVec);
    void QueryCompeteSliceByIdsExecuteSQL(std::shared_ptr<VirtualTraceDatabase> database,
                                          const std::vector<uint64_t> &sliceIds,
                                          std::vector<CompeteSliceDomain> &competeSliceVec);
    bool QuerySliceDetailInfoExecuteSQL(std::shared_ptr<VirtualTraceDatabase> database,
                                        const std::string &sliceId, CompeteSliceDomain &competeSliceDomain);
};
}
#endif // PROFILER_SERVER_OSRTAPI_H
