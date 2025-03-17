/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "QueryOverallMoreDetailsHandler.h"
#include "HandlerTest.cpp"

TEST_F(HandlerTest, QueryOverallMoreDetailsHandlerTestNormal)
{
    Dic::Module::Timeline::QueryOverallMoreDetailsHandler handler;
    std::unique_ptr<Dic::Protocol::Request> requestPtr =
        std::make_unique<Dic::Protocol::SystemViewOverallMoreDetailsRequest>();
    handler.HandleRequest(std::move(requestPtr));
}