/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "QueryOverallMoreDetailsHandler.h"
#include "HandlerTest.cpp"
#include "DataBaseManager.h"

using namespace Dic::Module::Timeline;
TEST_F(HandlerTest, QueryOverallMoreDetailsHandlerTestNormal)
{
    Dic::Module::Timeline::QueryOverallMoreDetailsHandler handler;
    auto requestPtr =
        std::make_unique<Dic::Protocol::SystemViewOverallMoreDetailsRequest>();
    requestPtr->params.rankId = "0";
    DataBaseManager::Instance().CreatTraceConnectionPool("0", "test");
    handler.HandleRequest(std::move(requestPtr));
    DataBaseManager::Instance().Clear();
}