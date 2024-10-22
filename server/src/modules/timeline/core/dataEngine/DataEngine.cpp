// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "pch.h"
#include "DataEngine.h"

namespace Dic::Module::Timeline {
void DataEngine::SetRepositoryFactory(std::shared_ptr<RepositoryFactoryInterface> respotoryFactoryInterface)
{
    respotoryFactory = std::move(respotoryFactoryInterface);
}

/**
 * 根据trackId查询简单算子
 * @param sliceQuery
 * @param sliceVec
 */
void DataEngine::QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery, std::vector<SliceDomain> &sliceVec)
{
    auto sliceRespo = respotoryFactory->GetSliceRespo(sliceQuery.metaType);
    if (sliceRespo == nullptr) {
        return;
    }
    sliceRespo->QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
}

void DataEngine::QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds)
{
    auto sliceRespo = respotoryFactory->GetSliceRespo(sliceQuery.metaType);
    if (sliceRespo == nullptr) {
        return;
    }
    sliceRespo->QuerySliceIdsByCat(sliceQuery, sliceIds);
}

uint64_t DataEngine::QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery)
{
    auto sliceRespo = respotoryFactory->GetSliceRespo(sliceQuery.metaType);
    if (sliceRespo == nullptr) {
        return 0;
    }
    const uint64_t count = sliceRespo->QueryPythonFunctionCountByTrackId(sliceQuery);
    return count;
}

void DataEngine::QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
    std::vector<CompeteSliceDomain> &sliceVec)
{
    auto sliceRespo = respotoryFactory->GetSliceRespo(sliceQuery.metaType);
    if (sliceRespo == nullptr) {
        return;
    }
    sliceRespo->QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
}

void DataEngine::QueryFlowPointByTimeRange(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    auto flowRespo = respotoryFactory->GetFlowRespo(flowQuery.metaType);
    if (flowRespo == nullptr) {
        return;
    }
    flowRespo->QueryFlowPointByTimeRange(flowQuery, flowPointVec);
}

void DataEngine::QueryFlowPointByFlowId(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    auto flowRespo = respotoryFactory->GetFlowRespo(flowQuery.metaType);
    if (flowRespo == nullptr) {
        return;
    }
    flowRespo->QueryFlowPointByFlowId(flowQuery, flowPointVec);
}

void DataEngine::QueryAllThreadInfo(const ThreadQuery &threadQuery,
    std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo)
{
    auto sliceRespo = respotoryFactory->GetSliceRespo(threadQuery.metaType);
    if (sliceRespo == nullptr) {
        return;
    }
    sliceRespo->QueryAllThreadInfo(threadQuery, threadInfo);
}

/**
 * 根据Id集合查询完整算子信息
 * @param sliceQuery
 * @param sliceIds
 * @param CompeteSliceVec
 */
void DataEngine::QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
    std::vector<CompeteSliceDomain> &competeSliceVec)
{
    auto sliceRespo = respotoryFactory->GetSliceRespo(sliceQuery.metaType);
    if (sliceRespo == nullptr) {
        return;
    }
    sliceRespo->QueryCompeteSliceByIds(sliceQuery, sliceIds, competeSliceVec);
}

void DataEngine::QueryFlowPointByCategory(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    auto textFlowRespo = respotoryFactory->GetFlowRespo(PROCESS_TYPE::TEXT);
    textFlowRespo->QueryFlowPointByCategory(flowQuery, flowPointVec);
    if (!std::empty(flowPointVec)) {
        return;
    }
    auto DbFlowRepo = respotoryFactory->GetFlowRespo(PROCESS_TYPE::DB);
    DbFlowRepo->QueryFlowPointByCategory(flowQuery, flowPointVec);
}

void DataEngine::QueryAllFlagSlice(const SliceQuery &sliceQuery, std::vector<CompeteSliceDomain> &competeSliceDomainVec)
{
    auto textRepo = respotoryFactory->GetSimulationSliceRespo(PROCESS_TYPE::TEXT);
    textRepo->QueryAllFlagSlice(sliceQuery, competeSliceDomainVec);
}
}
