/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_REPOSITORYINTERFACE_H
#define PROFILER_SERVER_REPOSITORYINTERFACE_H
#include <vector>
#include <set>
#include "DomainObject.h"
#include "DominQuery.h"
namespace Dic::Module::Timeline {
class RepositoryInterface {
public:
    ~RepositoryInterface() = default;
    /* *
     * 根据trackId查询简单算子
     * @param sliceQuery
     * @param sliceVec
     */
    virtual void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
        std::vector<SliceDomain> &sliceVec) = 0;
    /* *
     * 根据TrackId查询所有调用栈算子的id,返回结果升序
     * @param sliceQuery
     * @param sliceIds
     */
    virtual void QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds) = 0;

    /* *
     * 根据TrackId查询python function数据条数
     * @param sliceQuery
     * @return
     */
    virtual uint64_t QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery) = 0;

    /* *
     * 查询时间范围内所有算子（包括与范围相交的）
     * @param sliceQuery
     * @param sliceVec
     */
    virtual void QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
        std::vector<CompeteSliceDomain> &sliceVec) = 0;
};
}
#endif // PROFILER_SERVER_REPOSITORYINTERFACE_H
