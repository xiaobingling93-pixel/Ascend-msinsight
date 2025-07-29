//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_PYTHONGCREPO_H
#define PROFILER_SERVER_PYTHONGCREPO_H
#include "MstxEventsTable.h"
#include "EnumMstxEventTypeTable.h"
#include "StringIdsTable.h"
#include "SliceRepoInterface.h"
#include "PythonGCTable.h"

namespace Dic::Module::Timeline {
class PythonGcRepo : public IBaseSliceRepo {
public:
    void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
                                                std::vector<SliceDomain> &sliceVec) override;
    void QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
                                std::vector<CompeteSliceDomain> &competeSliceVec) override;
    bool QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;

protected:
    std::unique_ptr<PythonGCTable> table = std::make_unique<PythonGCTable>();
};
}
#endif // PROFILER_SERVER_PYTHONGCREPO_H
