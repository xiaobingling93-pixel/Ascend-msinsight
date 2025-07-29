//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
//

#ifndef PROFILER_SERVER_OSRTAPI_H
#define PROFILER_SERVER_OSRTAPI_H
#include "SliceRepoInterface.h"
#include "VirtualTraceDatabase.h"
namespace Dic::Module::Timeline {
class OSRTApiRepo : public IBaseSliceRepo {
public:
    void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
                                              std::vector<SliceDomain> &sliceVec) override;
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
