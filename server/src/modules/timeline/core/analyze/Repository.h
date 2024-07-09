/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_REPOSITORY_H
#define PROFILER_SERVER_REPOSITORY_H
#include "RepositoryInterface.h"
namespace Dic::Module::Timeline {
class Repository : public RepositoryInterface {
public:
    ~Repository() = default;
    /* *
     * 根据trackId查询简单算子
     * @param sliceQuery
     * @param sliceVec
     */
    void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
        std::vector<SliceDomain> &sliceVec) override;
    /* *
     * 根据TrackId查询所有调用栈算子的id,返回结果升序
     * @param sliceQuery
     * @param sliceIds
     */
    void QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds) override;

    /* *
     * 根据TrackId查询python function数据条数
     * @param sliceQuery
     * @return
     */
    uint64_t QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery) override;

    /* *
     * 查询时间范围内所有算子（包括与范围相交的）
     * @param sliceQuery
     * @param sliceVec
     */
    void QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
        std::vector<CompeteSliceDomain> &sliceVec) override;
};
}
#endif // PROFILER_SERVER_REPOSITORY_H
