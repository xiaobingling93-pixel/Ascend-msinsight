/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DataEngine.h"
#include "TrackInfoManager.h"
#include "CacheManager.h"
#include "RenderEngine.h"
#include "DominQuery.h"

using namespace Dic::Module::Timeline;
class RenderEngineTest : public ::testing::Test {
    void SetUp() override
    {
        TrackInfoManager::Instance().Reset();
        CacheManager::Instance().ClearAll();
    }

    void TearDown() override
    {
        TrackInfoManager::Instance().Reset();
        CacheManager::Instance().ClearAll();
    }
};

/**
 * 根据时间点查询算子，名字存在，但没有算子信息
 */
TEST_F(RenderEngineTest, TestFindSliceByTimePointNormal)
{
    const uint64_t expectTrackId = 8;
    const uint64_t expectId = 70;
    const uint32_t expectDepth = 2;
    class DataEngineMock : public DataEngine {
    public:
        bool QuerySliceByTimepointAndName(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override
        {
            competeSliceDomain.trackId = expectTrackId;
            competeSliceDomain.id = expectId;
            return true;
        }
    };
    SliceCacheManager &sliceCacheManager = SliceCacheManager::Instance();
    std::vector<SliceDomain> sliceVec;
    SliceDomain sliceDomain1;
    sliceDomain1.id = expectId;
    sliceDomain1.depth = expectDepth;
    sliceVec.emplace_back(sliceDomain1);
    sliceCacheManager.UpdateSliceCache("8", sliceVec);
    RenderEngine renderEngine;
    std::shared_ptr<DataEngineMock> dataEngineMock = std::make_unique<DataEngineMock>();
    renderEngine.SetDataEngineInterface(dataEngineMock);
    CompeteSliceDomain slice = renderEngine.FindSliceByTimePoint("", "", 0, "TEXT");
    EXPECT_EQ(slice.depth, expectDepth);
}

/**
 * 根据时间点查询算子，查询返回 false，打印日志中的特殊字符转义
 */
TEST_F(RenderEngineTest, TestFindSliceByTimePointTypeWrong)
{
    class DataEngineMock : public DataEngine {
    public:
        bool QuerySliceByTimepointAndName(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain) override
        {
            return false;
        }
    };
    RenderEngine renderEngine;
    std::shared_ptr<DataEngineMock> dataEngineMock = std::make_unique<DataEngineMock>();
    renderEngine.SetDataEngineInterface(dataEngineMock);
    CompeteSliceDomain slice = renderEngine.FindSliceByTimePoint("", "AAA\n%\t\\", 0, "TEXT");
}
