// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#ifndef PROFILER_SERVER_SLICERESPOTERFACE_H
#define PROFILER_SERVER_SLICERESPOTERFACE_H
#include <vector>
#include <unordered_map>
#include "DominQuery.h"
#include "DomainObject.h"
namespace Dic::Module::Timeline {
class SliceRepoInterface {
public:
    virtual ~SliceRepoInterface() = default;
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

    /* *
     * 查询所有泳道线程和进程信息
     * @param flowQuery
     * @param flowPointVec
     */
    virtual void QueryAllThreadInfo(const ThreadQuery &flowQuery,
        std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo) = 0;
    /* *
     * 根据Id集合查询完整算子信息
     * @param sliceQuery
     * @param sliceIds
     * @param CompeteSliceVec
     */
    virtual void QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
        std::vector<CompeteSliceDomain> &CompeteSliceVec) = 0;
};
}
#endif // PROFILER_SERVER_SLICERESPOTERFACE_H
