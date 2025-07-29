//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_MSTXREPO_H
#define PROFILER_SERVER_MSTXREPO_H
#include "MstxEventsTable.h"
#include "EnumMstxEventTypeTable.h"
#include "StringIdsTable.h"
#include "SliceRepoInterface.h"
namespace Dic::Module::Timeline {
class MstxRepo : public IBaseSliceRepo {
public:
    void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
        std::vector<SliceDomain> &sliceVec) override;
    void QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
        std::vector<CompeteSliceDomain> &CompeteSliceVec) override;
    bool QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;
protected:
    std::unique_ptr<MstxEventsTable> mstxEventsTable = std::make_unique<MstxEventsTable>();
    std::unique_ptr<EnumMstxEventTypeTable> enumMstxEventTypeTable = std::make_unique<EnumMstxEventTypeTable>();
    std::unique_ptr<StringIdsTable> stringIdsTable = std::make_unique<StringIdsTable>();
};
}
#endif // PROFILER_SERVER_MSTXREPO_H
