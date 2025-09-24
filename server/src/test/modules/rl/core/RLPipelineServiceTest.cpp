/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DataBaseManager.h"
#include "RLPipelineService.h"
#include "ParamsParser.h"
#include "RepositoryFactory.h"
#include "DataEngine.h"
#include "RenderEngine.h"


using namespace Dic::Module::FullDb;
using namespace Dic::Module::RL;
using namespace Dic::Module;
class RLPipelineServiceTest : public ::testing::Test {
    void SetUp() override
    {
        class DataEngineMock : public DataEngine {
        public:
            bool QuerySliceDetailInfoByNameList(const SliceQueryByNameList &params,
                                                std::vector<CompeteSliceDomain> &res) override
            {
                CompeteSliceDomain domain1;
                domain1.name = "generate_sequences";
                domain1.timestamp = 100;
                domain1.endTime = 1000;
                res.push_back(domain1);
                return true;
            }
        };
        auto renderEngine = RenderEngine::Instance();
        std::shared_ptr<DataEngineMock> dataEngineMock = std::make_unique<DataEngineMock>();
        renderEngine->SetDataEngineInterface(dataEngineMock);
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().CreateTraceConnectionPool("uboot14286042774212449010_0 0", "dbPath");
    }
};

TEST_F(RLPipelineServiceTest, GetPipelineInfoSuccess)
{
    Protocol::RLPipelineResponse response;
    bool res = Dic::Module::RL::RLPipelineService::Instance().GetPipelineInfo(response);
    const uint64_t expectMinTime = 100;
    const uint64_t expectMaxTime = 1000;
    const uint64_t expectSize = 1;
    EXPECT_EQ(res, true);
    EXPECT_EQ(response.body.minTime, expectMinTime);
    EXPECT_EQ(response.body.maxTime, expectMaxTime);
    EXPECT_EQ(response.body.taskData.size(), expectSize);
    EXPECT_EQ(response.body.stageTypeList.size(), expectSize);
    EXPECT_EQ(response.body.taskData[0].hostName, "uboot14286042774212449010");
}