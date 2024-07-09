/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "RepositoryInterface.h"
#include "SliceAnalyzer_mock_data.h"
#include "SliceAnalyzer.h"
#include "CacheManager.h"
using namespace Dic::TimeLine::SliceAnalyzer::Mock;
class SliceAnalyzerTest : public ::testing::Test {};
/**
 * 测试过滤pf的框选功能
 */
TEST_F(SliceAnalyzerTest, test_ComputeSliceDomainVecAndSelfTimeByTimeRange_filter_python_function)
{
    // 对Repository进行mock数据
    class RepositoryMock : public Dic::Module::Timeline::Repository {
    public:
        void QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
            std::vector<CompeteSliceDomain> &sliceVec) override
        {
            QueryCompeteSliceVecByTimeRangeAndTrackId_mock(sliceQuery, sliceVec);
        }
    };
    std::unique_ptr<Dic::Module::Timeline::RepositoryInterface> ptr = std::make_unique<RepositoryMock>();
    SliceAnalyzer sliceAnalyzer;
    sliceAnalyzer.SetRepository(std::move(ptr));
    SliceQuery sliceQuery = { nullptr, 3, 0, 23, 2 };
    SliceCacheFliterPythonMock();
    SliceCacheManager::Instance().PutPythonFunctionIdVec(std::to_string(sliceQuery.trackId), { 1 });
    SliceCacheManager::Instance().UpdatePythonFilterSet(std::to_string(sliceQuery.trackId), true);
    std::vector<CompeteSliceDomain> sliceDomainVec;
    std::map<std::string, uint64_t> selfTimeKeyValue;
    sliceAnalyzer.ComputeSliceDomainVecAndSelfTimeByTimeRange(sliceQuery, sliceDomainVec, selfTimeKeyValue);
    const uint64_t expectSize = 9;
    const uint64_t expectSlice2SelfTime = 6;
    const uint64_t expectSlice3SelfTime = 4;
    EXPECT_EQ(sliceDomainVec.size(), expectSize);
    EXPECT_EQ(sliceDomainVec.begin()->name, "slice2");
    EXPECT_EQ(selfTimeKeyValue["slice2"], expectSlice2SelfTime);
    EXPECT_EQ(selfTimeKeyValue["slice3"], expectSlice3SelfTime);
    CacheManager::Instance().ClearAll();
}

/**
 * 测试不过滤过滤pf的框选功能
 */
TEST_F(SliceAnalyzerTest, test_ComputeSliceDomainVecAndSelfTimeByTimeRange_not_filter_python_function)
{
    // 对Repository进行mock数据
    class RepositoryMock : public Dic::Module::Timeline::Repository {
    public:
        void QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
            std::vector<CompeteSliceDomain> &sliceVec) override
        {
            QueryCompeteSliceVecByTimeRangeAndTrackId_mock(sliceQuery, sliceVec);
        }
    };
    std::unique_ptr<Dic::Module::Timeline::RepositoryInterface> ptr = std::make_unique<RepositoryMock>();
    SliceAnalyzer sliceAnalyzer;
    sliceAnalyzer.SetRepository(std::move(ptr));
    SliceQuery sliceQuery = { nullptr, 3, 0, 23, 2 };
    SliceCacheNotFliterPythonMock();
    std::vector<CompeteSliceDomain> sliceDomainVec;
    std::map<std::string, uint64_t> selfTimeKeyValue;
    sliceAnalyzer.ComputeSliceDomainVecAndSelfTimeByTimeRange(sliceQuery, sliceDomainVec, selfTimeKeyValue);
    const uint64_t expectSize = 10;
    const uint64_t expectSlice1SelfTime = 6;
    const uint64_t expectSlice2SelfTime = 6;
    const uint64_t expectSlice3SelfTime = 4;
    EXPECT_EQ(sliceDomainVec.size(), expectSize);
    EXPECT_EQ(sliceDomainVec.begin()->name, "slice1");
    EXPECT_EQ(selfTimeKeyValue["slice1"], expectSlice1SelfTime);
    EXPECT_EQ(selfTimeKeyValue["slice2"], expectSlice2SelfTime);
    EXPECT_EQ(selfTimeKeyValue["slice3"], expectSlice3SelfTime);
    CacheManager::Instance().ClearAll();
}

/**
 * 测试简单算子过期后不过滤过滤pf的框选功能
 */
TEST_F(SliceAnalyzerTest, test_ComputeSelfTimeByTimeRange_cache_isExpire_not_filter_python_function)
{
    // 对Repository进行mock数据
    class RepositoryMock : public Dic::Module::Timeline::Repository {
    public:
        void QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
            std::vector<CompeteSliceDomain> &sliceVec) override
        {
            QueryCompeteSliceVecByTimeRangeAndTrackId_mock(sliceQuery, sliceVec);
        }
        void QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
            std::vector<SliceDomain> &sliceVec) override
        {
            QuerySimpleSliceWithOutNameByTrackId_mock(sliceQuery, sliceVec);
        }
    };
    std::unique_ptr<Dic::Module::Timeline::RepositoryInterface> ptr = std::make_unique<RepositoryMock>();
    SliceAnalyzer sliceAnalyzer;
    sliceAnalyzer.SetRepository(std::move(ptr));
    SliceQuery sliceQuery = { nullptr, 3, 0, 23, 2 };
    std::vector<CompeteSliceDomain> sliceDomainVec;
    std::map<std::string, uint64_t> selfTimeKeyValue;
    sliceAnalyzer.ComputeSliceDomainVecAndSelfTimeByTimeRange(sliceQuery, sliceDomainVec, selfTimeKeyValue);
    const uint64_t expectSize = 10;
    const uint64_t expectSlice1SelfTime = 6;
    const uint64_t expectSlice2SelfTime = 6;
    const uint64_t expectSlice3SelfTime = 4;
    EXPECT_EQ(sliceDomainVec.size(), expectSize);
    EXPECT_EQ(sliceDomainVec.begin()->name, "slice1");
    EXPECT_EQ(selfTimeKeyValue["slice1"], expectSlice1SelfTime);
    EXPECT_EQ(selfTimeKeyValue["slice2"], expectSlice2SelfTime);
    EXPECT_EQ(selfTimeKeyValue["slice3"], expectSlice3SelfTime);
    CacheManager::Instance().ClearAll();
}
