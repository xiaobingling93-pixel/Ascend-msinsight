/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "QueryExpAnaAICoreFreqHandler.h"
#include "HandlerTest.cpp"

class QueryExpAnaAICoreFreqHandlerTest : HandlerTest {
};

TEST_F(HandlerTest, QueryExpAnaAICoreFreqHandlerTestNormal)
{
    Dic::Module::Timeline::QueryExpAnaAICoreFreqHandler handler;
    std::unique_ptr<Dic::Protocol::Request> requestPtr = std::make_unique<Dic::Protocol::ExpAnaAICoreFreqRequest>();
    handler.HandleRequest(std::move(requestPtr));
}
