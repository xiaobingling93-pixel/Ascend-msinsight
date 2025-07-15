/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <SetCardAliasHandler.h>
#include <gtest/gtest.h>
#include "CreateCurveHandler.h"
#include "SearchSliceHandler.h"
#include "HandlerTest.cpp"

class SetCardAliasHandlerTest : HandlerTest {
};

TEST_F(HandlerTest, SetCardAliasHandlerTestNotVariable)
{
    Dic::Module::Timeline::SetCardAliasHandler handler;
    std::unique_ptr<Dic::Protocol::Request> requestPtr = std::make_unique<Dic::Protocol::SetCardAliasRequest>();
    EXPECT_EQ(handler.HandleRequest(std::move(requestPtr)), false);
}

TEST_F(HandlerTest, SetCardAliasHandlerTestInvalidParam)
{
    Dic::Module::Timeline::SetCardAliasHandler handler;
    std::unique_ptr<Dic::Protocol::SetCardAliasRequest> requestPtr =
        std::make_unique<Dic::Protocol::SetCardAliasRequest>();
    requestPtr->command = "unit/setCardAlias";
    requestPtr->projectName = "A";
    requestPtr->params.cardAlias = "alias";
    requestPtr->params.rankId = "$./|";
    EXPECT_EQ(handler.HandleRequest(std::move(requestPtr)), false);
}
TEST_F(HandlerTest, TestCreateCurveHandler)
{
    class MockOpenApi : public Dic::Module::IE::ServitizationOpenApi {
    public:
        bool CreateCurve(const std::string& fileId, const std::string& curve) override
        {
            return true;
        }
    };
    Dic::Module::Timeline::CreateCurveHandler handler;
    handler.openApi = std::make_shared<MockOpenApi>();
    auto requestPtr = std::make_unique<Dic::Protocol::CreateCurveRequest>();
    requestPtr->params.x = "Communication(Not Overlapped)";
    bool result = handler.HandleRequest(std::move(requestPtr));
    EXPECT_EQ(result, false);
    auto requestPtr2 = std::make_unique<Dic::Protocol::CreateCurveRequest>();
    requestPtr2->params.x = "kkkkkkkkk";
    requestPtr2->params.type = "1";
    result = handler.HandleRequest(std::move(requestPtr2));
    EXPECT_EQ(result, true);
    auto requestPtr3 = std::make_unique<Dic::Protocol::CreateCurveRequest>();
    requestPtr3->params.x = "kkkkkkkkk";
    requestPtr3->params.type = "2";
    result = handler.HandleRequest(std::move(requestPtr3));
    EXPECT_EQ(result, true);
}