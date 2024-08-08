// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#ifndef PROFILER_SERVER_TEXTREPOSITORY_H
#define PROFILER_SERVER_TEXTREPOSITORY_H
#include "SliceRepoInterface.h"
namespace Dic::Module::Timeline {
class TextRepository : public SliceRepoInterface {
public:
    ~TextRepository() override = default;
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

    /* *
     * 查询时间范围内的所有连线点
     * @param flowQuery
     * @param flowPointVec
     */
    void QueryFlowPointByTimeRange(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) override;

    /* *
     * 根据连线点id查询所有连线点
     * @param flowQuery
     * @param flowPointVec
     */
    void QueryFlowPointByFlowId(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) override;

    void QueryAllThreadInfo(const ThreadQuery &flowQuery,
        std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo) override;

    /* *
     * 根据Id集合查询完整算子信息
     * @param sliceQuery
     * @param sliceIds
     * @param CompeteSliceVec
     */
    void QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
        std::vector<CompeteSliceDomain> &CompeteSliceVec) override;
};
}
#endif // PROFILER_SERVER_TEXTREPOSITORY_H
