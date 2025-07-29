//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_OVERLAPANSREPO_H
#define PROFILER_SERVER_OVERLAPANSREPO_H
#include "SliceRepoInterface.h"
namespace Dic::Module::Timeline {
class OverlapAnsRepo : public IBaseSliceRepo {
public:
    void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
        std::vector<SliceDomain> &sliceVec) override;
    void QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
        std::vector<CompeteSliceDomain> &CompeteSliceVec) override;
    bool QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;

    static int GetTypeByName(const std::string &name);
};
}
#endif // PROFILER_SERVER_OVERLAPANSREPO_H
