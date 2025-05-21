/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include <utility>
#include "CurveGroupHandler.h"
using namespace Dic::Module::IE;
class CurveGroupHandlerTest : public ::testing::Test {
protected:
    class MockRepo : public Dic::Module::IE::CurveRepo {
    public:
        std::vector<std::string> QueryAllViews(const std::string& fileId)
        {
            std::vector<std::string> res;
            res.emplace_back("llllllllllllllll");
            return res;
        }
    };

    class MockCurveGroupHandler : public Dic::Module::IE::CurveGroupHandler {
    public:
        void SetRepo(std::shared_ptr<CurveRepo> mockRepo)
        {
            repo = std::move(mockRepo);
        }
    };
};

TEST_F(CurveGroupHandlerTest, TestQueryAllViews)
{
    std::shared_ptr<MockRepo> mockRepo = std::make_shared<MockRepo>();
    std::shared_ptr<MockCurveGroupHandler> handler = std::make_shared<MockCurveGroupHandler>();
    std::unique_ptr<Dic::Protocol::IEGroupRequest> request = std::make_unique<Dic::Protocol::IEGroupRequest>();
    handler->SetRepo(mockRepo);
    auto res = handler->HandleRequest(std::move(request));
    EXPECT_EQ(res, true);
}
