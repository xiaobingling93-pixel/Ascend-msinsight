//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_PYTHONAPIREPO_H
#define PROFILER_SERVER_PYTHONAPIREPO_H
#include "SliceRepoInterface.h"
#include "PytorchApiTable.h"
#include "PytorchCallchainsTable.h"
#include "StringIdsTable.h"
namespace Dic::Module::Timeline {
class PythonApiRepo : public IBaseSliceRepo, public IPythonFuncSlice, public IFindSliceByTimepointAndName {
public:
    void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
        std::vector<SliceDomain> &sliceVec) override;
    void QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds) override;
    uint64_t QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery) override;
    void QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
        std::vector<CompeteSliceDomain> &CompeteSliceVec) override;
    bool QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;

    ~PythonApiRepo() override = default;

    bool QuerySliceByTimepointAndName(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;

protected:
    std::unique_ptr<PytorchApiTable> pytorchApiTable = std::make_unique<PytorchApiTable>();
    std::unique_ptr<StringIdsTable> stringIdsTable = std::make_unique<StringIdsTable>();
    std::unique_ptr<PytorchCallchainsTable> pytorchCallchainsTable = std::make_unique<PytorchCallchainsTable>();

    std::string QuerySliceCallStack(const SliceQuery &sliceQuery, const PytorchApiPO &target);

    void
    QuerySliceArgs(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain, const PytorchApiPO &target);
private:
    const std::string pythonApiTid = "pytorch";
};
}
#endif // PROFILER_SERVER_PYTHONAPIREPO_H
