/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "CommunicationProtocolRequest.h"
#include "../../timeline/handler/HandlerTest.cpp"
#include "CommunicationOperatorListsHandler.h"
#include "BandwidthHandler.h"
#include "CommunicationOperatorDetailsHandler.h"
#include "DistributionHandler.h"
#include "GroupHandler.h"
#include "IterationsHandler.h"
#include "MatrixListHandler.h"
#include "MatrixSortOpNamesHandler.h"
#include "OperatorNamesHandler.h"
#include "RanksHandler.h"
#include "CommunicationAdvisorHandler.h"


using namespace Dic::Module;
using namespace Dic::Module::Communication;

TEST_F(HandlerTest, QueryCommunicationOperatorListsWithParamError)
{
    auto request = std::make_unique<DurationListRequest>();
    request->params.iterationId = ";";
    request->params.operatorName = "opName";
    request->params.stage = "1";
    CommunicationOperatorListsHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}


TEST_F(HandlerTest, QueryCommunicationOperatorListsWithExeSqlFail)
{
    auto request = std::make_unique<DurationListRequest>();
    request->params.iterationId = "1";
    request->params.operatorName = "opName";
    request->params.stage = "1";
    CommunicationOperatorListsHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(HandlerTest, QueryBandwidthHandlerParamError)
{
    auto request = std::make_unique<BandwidthDataRequest>();
    request->params.iterationId = ";";
    request->params.operatorName = "opName";
    request->params.stage = "1";
    BandwidthHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryBandwidthHandlerWithExeSqlFail)
{
    auto request = std::make_unique<BandwidthDataRequest>();
    request->params.iterationId = "1";
    request->params.operatorName = "opName";
    request->params.stage = "1";
    BandwidthHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, CommunicationOperatorDetailsParamError)
{
    auto request = std::make_unique<OperatorDetailsRequest>();
    request->params.iterationId = ";";
    request->params.orderBy = "orderBy";
    request->params.order = "order";
    request->params.stage = "1";
    CommunicationOperatorDetailsHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, CommunicationOperatorDetailsWithExeSqlFail)
{
    auto request = std::make_unique<OperatorDetailsRequest>();
    request->params.iterationId = "1";
    request->params.orderBy = "orderBy";
    request->params.order = "order";
    request->params.stage = "1";
    CommunicationOperatorDetailsHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, DistributionHandlerParamError)
{
    auto request = std::make_unique<DistributionDataRequest>();
    request->params.iterationId = ";";
    request->params.operatorName = "name";
    request->params.transportType = "type";
    request->params.stage = "1";
    DistributionHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, DistributionHandlerWithExeSqlFail)
{
    auto request = std::make_unique<DistributionDataRequest>();
    request->params.iterationId = "1";
    request->params.operatorName = "name";
    request->params.transportType = "type";
    request->params.stage = "1";
    DistributionHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, GroupHandlerParamError)
{
    auto request = std::make_unique<MatrixGroupRequest>();
    request->params.iterationId = ";";
    GroupHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, GroupHandlerWithExeSqlFail)
{
    auto request = std::make_unique<MatrixGroupRequest>();
    request->params.iterationId = "1";
    GroupHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(HandlerTest, IterationsHandlerWithExeSqlFail)
{
    auto request = std::make_unique<IterationsRequest>();
    IterationsHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(HandlerTest, MatrixListHandlerParamError)
{
    auto request = std::make_unique<MatrixBandwidthRequest>();
    request->params.iterationId = ";";
    request->params.operatorName = "name";
    request->params.stage = "1";
    MatrixListHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, MatrixListHandlerWithExeSqlFail)
{
    auto request = std::make_unique<MatrixBandwidthRequest>();
    request->params.iterationId = "1";
    request->params.operatorName = "name";
    request->params.stage = "1";
    MatrixListHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(HandlerTest, MatrixSortOpNamesHandlerWithExeSqlFail)
{
    auto request = std::make_unique<MatrixSortOpNamesRequest>();
    MatrixSortOpNamesHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, OperatorNamesHandlerParamError)
{
    auto request = std::make_unique<OperatorNamesRequest>();
    request->params.iterationId = ";";
    request->params.stage = "1";
    OperatorNamesHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, OperatorNamesHandlerWithExeSqlFail)
{
    auto request = std::make_unique<OperatorNamesRequest>();
    request->params.iterationId = "1";
    request->params.stage = "1";
    OperatorNamesHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, RanksHandlerParamError)
{
    auto request = std::make_unique<RanksRequest>();
    request->params.iterationId = ";";
    RanksHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, RanksHandlerWithExeSqlFail)
{
    auto request = std::make_unique<RanksRequest>();
    request->params.iterationId = "1";
    RanksHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, AdvisorHandlerNormalTest)
{
    std::unique_ptr<CommunicationAdvisorRequest> request = std::make_unique<CommunicationAdvisorRequest>();
    CommunicationAdvisorHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_TRUE(result);
}