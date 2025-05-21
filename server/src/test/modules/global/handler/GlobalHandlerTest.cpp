/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "../../timeline/handler/HandlerTest.cpp"
#include "GlobalProtocolRequest.h"
#include "GetProjectExplorerInfoHandler.h"
#include "SetBaselineHandler.h"
#include "GetModuleConfigHandler.h"
#include "ClearProjectExplorerHandler.h"

using namespace Dic::Module;
TEST_F(HandlerTest, GetProjectExplorerInfoWithEmptyResult)
{
    auto request = std::make_unique<Dic::Module::ProjectExplorerInfoGetRequest>();
    Dic::Module::GetProjectExplorerInfoHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(HandlerTest, SetBaselineHandler)
{
    auto request = std::make_unique<BaselineSettingRequest>();
    request->params.baselineClusterPath = "COMPARE";
    request->params.projectName = "test";
    request->params.filePath = "COMPARE";
    request->params.currentClusterPath = "Baseline";
    SetBaselineHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, GetModuleConfigHandler)
{
    auto request = std::make_unique<BaselineSettingRequest>();
    GetModuleConfigHandler handler;
    EXPECT_EQ(handler.HandleRequest(std::move(request)), true);
}

TEST_F(HandlerTest, ClearProjectExplorerHandler)
{
    auto request = std::make_unique<ProjectExplorerInfoClearRequest>();
    ClearProjectExplorerHandler handler;
    EXPECT_EQ(handler.HandleRequest(std::move(request)), true);
}