/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DataEngine.h"
#include "RepositoryFactory.h"
#include "HcclRepo.h"
#include "TrackInfoManager.h"
#include "DominQuery.h"

using namespace Dic::Module::Timeline;
class DataEngineTest : public ::testing::Test {
    void SetUp() override
    {
        TrackInfoManager::Instance().Reset();
    }

    void TearDown() override
    {
        TrackInfoManager::Instance().Reset();
    }

protected:
    class SliceRepoMock : public Dic::Module::Timeline::HcclRepo {
    public:
        void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
            std::vector<SliceDomain> &sliceVec) override
        {
            SliceDomain sliceDomain1;
            SliceDomain sliceDomain2;
            SliceDomain sliceDomain3;
            const uint64_t expectStart1 = 10;
            const uint64_t expectId1 = 10;
            const uint64_t expectStart2 = 7;
            const uint64_t expectId2 = 11;
            const uint64_t expectStart3 = 7;
            const uint64_t expectId3 = 12;
            sliceDomain1.timestamp = expectStart1;
            sliceDomain1.id = expectId1;
            sliceDomain2.timestamp = expectStart2;
            sliceDomain2.id = expectId2;
            sliceDomain3.timestamp = expectStart3;
            sliceDomain3.id = expectId3;
            sliceVec.emplace_back(sliceDomain1);
            sliceVec.emplace_back(sliceDomain2);
            sliceVec.emplace_back(sliceDomain3);
        }
    };

    class RespotoryFactoryMock : public Dic::Module::Timeline::RepositoryFactory {
    public:
        std::shared_ptr<SliceRepoInterface> GetSliceRespo(PROCESS_TYPE)override
        {
            std::shared_ptr<SliceRepoInterface> res = std::make_shared<SliceRepoMock>();
            return res;
        }
    };
};

TEST_F(DataEngineTest, QueryAllThreadInfoNormalTest)
{
    std::unordered_map<uint64_t, std::pair<std::string, std::string>> threadInfo;
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        ThreadQuery threadQuery;
        dataEngine.QueryAllThreadInfo(threadQuery, threadInfo);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        threadQuery.metaType = PROCESS_TYPE::CANN_API;
        dataEngine.QueryAllThreadInfo(threadQuery, threadInfo);
        threadQuery.metaType = PROCESS_TYPE::MS_TX;
        dataEngine.QueryAllThreadInfo(threadQuery, threadInfo);
        threadQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QueryAllThreadInfo(threadQuery, threadInfo);
        threadQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QueryAllThreadInfo(threadQuery, threadInfo);
        threadQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QueryAllThreadInfo(threadQuery, threadInfo);
        threadQuery.metaType = PROCESS_TYPE::HCCL;
        dataEngine.QueryAllThreadInfo(threadQuery, threadInfo);
        threadQuery.metaType = PROCESS_TYPE::NONE;
        dataEngine.QueryAllThreadInfo(threadQuery, threadInfo);
        threadQuery.metaType = PROCESS_TYPE::TEXT;
        dataEngine.QueryAllThreadInfo(threadQuery, threadInfo);
    });
}

TEST_F(DataEngineTest, QuerySimpleSliceWithOutNameByTrackIdTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        SliceQuery sliceQuery;
        std::vector<SliceDomain> sliceVec;
        dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::CANN_API;
        dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::MS_TX;
        dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::HCCL;
        dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::NONE;
        dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::TEXT;
        dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
    });
}

TEST_F(DataEngineTest, QueryPythonFunctionCountByTrackIdTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        SliceQuery sliceQuery;
        dataEngine.QueryPythonFunctionCountByTrackId(sliceQuery);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::CANN_API;
        dataEngine.QueryPythonFunctionCountByTrackId(sliceQuery);
        sliceQuery.metaType = PROCESS_TYPE::MS_TX;
        dataEngine.QueryPythonFunctionCountByTrackId(sliceQuery);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QueryPythonFunctionCountByTrackId(sliceQuery);
        sliceQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QueryPythonFunctionCountByTrackId(sliceQuery);
        sliceQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QueryPythonFunctionCountByTrackId(sliceQuery);
        sliceQuery.metaType = PROCESS_TYPE::HCCL;
        dataEngine.QueryPythonFunctionCountByTrackId(sliceQuery);
        sliceQuery.metaType = PROCESS_TYPE::NONE;
        dataEngine.QueryPythonFunctionCountByTrackId(sliceQuery);
        sliceQuery.metaType = PROCESS_TYPE::TEXT;
        dataEngine.QueryPythonFunctionCountByTrackId(sliceQuery);
    });
}

TEST_F(DataEngineTest, QuerySliceIdsByCatTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        SliceQuery sliceQuery;
        std::vector<uint64_t> sliceIds;
        dataEngine.QuerySliceIdsByCat(sliceQuery, sliceIds);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::CANN_API;
        dataEngine.QuerySliceIdsByCat(sliceQuery, sliceIds);
        sliceQuery.metaType = PROCESS_TYPE::MS_TX;
        dataEngine.QuerySliceIdsByCat(sliceQuery, sliceIds);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QuerySliceIdsByCat(sliceQuery, sliceIds);
        sliceQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QuerySliceIdsByCat(sliceQuery, sliceIds);
        sliceQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QuerySliceIdsByCat(sliceQuery, sliceIds);
        sliceQuery.metaType = PROCESS_TYPE::HCCL;
        dataEngine.QuerySliceIdsByCat(sliceQuery, sliceIds);
        sliceQuery.metaType = PROCESS_TYPE::NONE;
        dataEngine.QuerySliceIdsByCat(sliceQuery, sliceIds);
        sliceQuery.metaType = PROCESS_TYPE::TEXT;
        dataEngine.QuerySliceIdsByCat(sliceQuery, sliceIds);
    });
}

TEST_F(DataEngineTest, QueryCompeteSliceVecByTimeRangeAndTrackIdTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        SliceQuery sliceQuery;
        std::vector<CompeteSliceDomain> sliceVec;
        dataEngine.QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::CANN_API;
        dataEngine.QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::MS_TX;
        dataEngine.QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::HCCL;
        dataEngine.QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::NONE;
        dataEngine.QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::TEXT;
        dataEngine.QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, sliceVec);
    });
}

TEST_F(DataEngineTest, QueryFlowPointByTimeRangeTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        FlowQuery sliceQuery;
        std::vector<FlowPoint> sliceVec;
        dataEngine.QueryFlowPointByTimeRange(sliceQuery, sliceVec);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::CANN_API;
        dataEngine.QueryFlowPointByTimeRange(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::MS_TX;
        dataEngine.QueryFlowPointByTimeRange(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QueryFlowPointByTimeRange(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QueryFlowPointByTimeRange(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QueryFlowPointByTimeRange(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::HCCL;
        dataEngine.QueryFlowPointByTimeRange(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::NONE;
        dataEngine.QueryFlowPointByTimeRange(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::TEXT;
        dataEngine.QueryFlowPointByTimeRange(sliceQuery, sliceVec);
    });
}

TEST_F(DataEngineTest, QueryFlowPointByFlowIdTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        FlowQuery sliceQuery;
        std::vector<FlowPoint> sliceVec;
        dataEngine.QueryFlowPointByFlowId(sliceQuery, sliceVec);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::CANN_API;
        dataEngine.QueryFlowPointByFlowId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::MS_TX;
        dataEngine.QueryFlowPointByFlowId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QueryFlowPointByFlowId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QueryFlowPointByFlowId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QueryFlowPointByFlowId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::HCCL;
        dataEngine.QueryFlowPointByFlowId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::NONE;
        dataEngine.QueryFlowPointByFlowId(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::TEXT;
        dataEngine.QueryFlowPointByFlowId(sliceQuery, sliceVec);
    });
}

TEST_F(DataEngineTest, QueryCompeteSliceByIdsTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        SliceQuery sliceQuery;
        std::vector<uint64_t> sliceIds;
        std::vector<CompeteSliceDomain> sliceVec;
        dataEngine.QueryCompeteSliceByIds(sliceQuery, sliceIds, sliceVec);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::CANN_API;
        dataEngine.QueryCompeteSliceByIds(sliceQuery, sliceIds, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::MS_TX;
        dataEngine.QueryCompeteSliceByIds(sliceQuery, sliceIds, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QueryCompeteSliceByIds(sliceQuery, sliceIds, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QueryCompeteSliceByIds(sliceQuery, sliceIds, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QueryCompeteSliceByIds(sliceQuery, sliceIds, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::HCCL;
        dataEngine.QueryCompeteSliceByIds(sliceQuery, sliceIds, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::NONE;
        dataEngine.QueryCompeteSliceByIds(sliceQuery, sliceIds, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::TEXT;
        dataEngine.QueryCompeteSliceByIds(sliceQuery, sliceIds, sliceVec);
    });
}

TEST_F(DataEngineTest, QuerySliceDetailInfoTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        SliceQuery sliceQuery;
        CompeteSliceDomain sliceVec;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::CANN_API;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::MS_TX;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::HCCL;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::NONE;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::TEXT;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
    });
}

TEST_F(DataEngineTest, QuerySliceDetailInfoFailTest)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        SliceQuery sliceQuery;
        sliceQuery.sliceId = "\n\t\bAA\v";
        CompeteSliceDomain sliceVec;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
        sliceQuery.metaType = PROCESS_TYPE::OVERLAP_ANALYSIS;
        dataEngine.QuerySliceDetailInfo(sliceQuery, sliceVec);
    });
}

TEST_F(DataEngineTest, QueryAllFlagSliceTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        SliceQuery sliceQuery;
        std::vector<CompeteSliceDomain> sliceVec;
        dataEngine.QueryAllFlagSlice(sliceQuery, sliceVec);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        dataEngine.QueryAllFlagSlice(sliceQuery, sliceVec);
    });
}

TEST_F(DataEngineTest, QueryFlowPointByCategoryTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        FlowQuery sliceQuery;
        std::vector<FlowPoint> sliceVec;
        dataEngine.QueryFlowPointByCategory(sliceQuery, sliceVec);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        dataEngine.QueryFlowPointByCategory(sliceQuery, sliceVec);
    });
}

TEST_F(DataEngineTest, QuerySliceByTimepointAndNameTestWithOutFactory)
{
    EXPECT_NO_THROW({
        Dic::Module::Timeline::DataEngine dataEngine;
        SliceQuery sliceQuery;
        CompeteSliceDomain sliceDomain;
        dataEngine.QuerySliceByTimepointAndName(sliceQuery, sliceDomain);
        std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
            std::make_shared<Dic::Module::Timeline::RepositoryFactory>();
        dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
        sliceQuery.metaType = PROCESS_TYPE::ASCEND_HARDWARE;
        dataEngine.QuerySliceByTimepointAndName(sliceQuery, sliceDomain);
        sliceQuery.metaType = PROCESS_TYPE::API;
        dataEngine.QuerySliceByTimepointAndName(sliceQuery, sliceDomain);
    });
}

TEST_F(DataEngineTest, QuerySimpleSliceWithOutNameByTrackId)
{
    Dic::Module::Timeline::DataEngine dataEngine;
    std::shared_ptr<Dic::Module::Timeline::RepositoryFactoryInterface> repositoryFactoryInterface =
        std::make_shared<RespotoryFactoryMock>();
    dataEngine.SetRepositoryFactory(repositoryFactoryInterface);
    const SliceQuery sliceQuery;
    std::vector<SliceDomain> sliceVec;
    dataEngine.QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
    const uint64_t expectSize = 3;
    const uint64_t first = 0;
    const uint64_t third = 2;
    EXPECT_EQ(sliceVec.size(), expectSize);
    const uint64_t firstTime = 7;
    EXPECT_EQ(sliceVec[first].timestamp, firstTime);
    const uint64_t firstId = 11;
    EXPECT_EQ(sliceVec[first].id, firstId);
    const uint64_t thirdTime = 10;
    EXPECT_EQ(sliceVec[third].timestamp, thirdTime);
    const uint64_t thirdId = 10;
    EXPECT_EQ(sliceVec[third].id, thirdId);
}