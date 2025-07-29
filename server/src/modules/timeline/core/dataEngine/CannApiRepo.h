//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_CANNAPIREPO_H
#define PROFILER_SERVER_CANNAPIREPO_H
#include "CannApiTable.h"
#include "StringIdsTable.h"
#include "EnumApiTypeTable.h"
#include "SliceRepoInterface.h"
namespace Dic::Module::Timeline {
class CannApiRepo : public IBaseSliceRepo, public IFindSliceByNameList {
public:
    void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
        std::vector<SliceDomain> &sliceVec) override;
    void QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
        std::vector<CompeteSliceDomain> &CompeteSliceVec) override;
    bool QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;
    bool QuerySliceDetailInfoByNameList(const SliceQueryByNameList &params,
                                        std::vector<CompeteSliceDomain> &res) override;
    void SetCannApiTable(std::unique_ptr<CannApiTable>);

protected:
    std::unique_ptr<EnumApiTypeTable> apiTypeTable = std::make_unique<EnumApiTypeTable>();
    std::unique_ptr<CannApiTable> cannApiTable = std::make_unique<CannApiTable>();
    std::unique_ptr<StringIdsTable> stringIdsTable = std::make_unique<StringIdsTable>();
};
}
#endif // PROFILER_SERVER_CANNAPIREPO_H
