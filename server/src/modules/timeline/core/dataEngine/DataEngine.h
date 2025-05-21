//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_DATAENGINE_H
#define PROFILER_SERVER_DATAENGINE_H
#include "RepositoryFactoryInterface.h"
#include "DataEngineInterface.h"

namespace Dic::Module::Timeline {
class DataEngine : public DataEngineInterface {
public:
    static std::shared_ptr<DataEngine> Instance()
    {
        static std::shared_ptr<DataEngine> instance = std::make_shared<DataEngine>();
        return instance;
    }
    DataEngine() = default;
    DataEngine(DataEngine &) = delete;
    DataEngine &operator = (DataEngine &) = delete;
    DataEngine(DataEngine &&) = delete;
    DataEngine &operator = (DataEngine &&) = delete;
    ~DataEngine() override = default;
    void SetRepositoryFactory(std::shared_ptr<RepositoryFactoryInterface>) override;
    void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
        std::vector<SliceDomain> &sliceVec) override;
    void QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds) override;
    uint64_t QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery) override;
    void QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
        std::vector<CompeteSliceDomain> &sliceVec) override;
    void QueryFlowPointByTimeRange(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) override;
    void QueryFlowPointByFlowId(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) override;
    void QueryAllThreadInfo(const ThreadQuery &flowQuery,
        std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo) override;
    void QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
        std::vector<CompeteSliceDomain> &CompeteSliceVec) override;
    void QueryFlowPointByCategory(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) override;
    void QueryAllFlagSlice(const SliceQuery &sliceQuery, std::vector<CompeteSliceDomain> &competeSliceDomain) override;
    bool QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;

    bool QuerySliceByTimepointAndName(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override;
    bool QuerySliceDetailInfoByNameList(const SliceQueryByNameList &params,
                                        std::vector<CompeteSliceDomain> &res) override;
private:
    std::shared_ptr<RepositoryFactoryInterface> respotoryFactory = nullptr;
};
}


#endif // PROFILER_SERVER_DATAENGINE_H
