/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "QuerySystemViewOverallHandler.h"
#include "HandlerTest.cpp"

TEST_F(HandlerTest, QuerySystemViewOverallHandlerTestNormal)
{
    Dic::Module::Timeline::QuerySystemViewOverallHandler handler;
    std::unique_ptr<Dic::Protocol::Request> requestPtr = std::make_unique<Dic::Protocol::SystemViewOverallRequest>();
    handler.HandleRequest(std::move(requestPtr));
}