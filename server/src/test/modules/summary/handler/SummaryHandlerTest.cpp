/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SummaryProtocolRequest.h"
#include "../../timeline/handler/HandlerTest.cpp"
#include "QueryFwdBwdTimelineHandler.h"
#include "QueryCommunicationDetailHandler.h"
#include "QueryComputeDetailInfoHandler.h"
#include "QueryParallelStrategyConfigHandler.h"
#include "RankAndBubbleTimeHandler.h"
#include "SetParallelStrategyConfigHandler.h"
#include "StageAndBubbleTimeHandler.h"
#include "QueryParallelismArrangementHandler.h"
#include "QueryParallelismPerformanceHandler.h"
#include "StageHandler.h"
#include "StepHandler.h"
#include "SummaryTopRankHandler.h"
#include "ImportExpertDataHandler.h"
#include "QueryExpertHotspotHandler.h"

using namespace Dic::Module;
using namespace Dic::Module::Summary;

const int NUMBER_FIVE_HUNDRED = 500;
const int NUMBER_TEN = 10;
const int NUMBER_ONE = 1;

TEST_F(HandlerTest, QueryFwdBwdTimelineHandlerHandleRequestReturnFalseWhenWrongParameter)
{
    auto request = std::make_unique<PipelineFwdBwdTimelineRequest>();
    request->params.stageId = "";
    request->params.stepId = "";
    QueryFwdBwdTimelineHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryFwdBwdTimelineHandlerHandleRequestReturnTrueWhenNormalParameter)
{
    auto request = std::make_unique<PipelineFwdBwdTimelineRequest>();
    request->params.stageId = "(0)";
    request->params.stepId = "1";
    QueryFwdBwdTimelineHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(HandlerTest, QueryCommunicationDetailHandlerWithExecuteSqlFail)
{
    auto request = std::make_unique<CommunicationDetailRequest>();
    request->params.rankId = "100";
    request->params.currentPage = NUMBER_ONE;
    request->params.pageSize = NUMBER_TEN;
    request->params.orderBy = "orderBy";
    request->params.order = "order";
    QueryCommunicationDetailHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryComputeDetailInfoHandlerWithExecuteSqlFail)
{
    auto request = std::make_unique<ComputeDetailRequest>();
    request->params.rankId = "100";
    request->params.currentPage = NUMBER_ONE;
    request->params.pageSize = NUMBER_TEN;
    request->params.orderBy = "orderBy";
    request->params.order = "order";
    request->params.timeFlag = "flag";
    QueryComputeDetailInfoHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryParallelStrategyConfigHandlerWithExecuteSqlFail)
{
    auto request = std::make_unique<QueryParallelStrategyRequest>();
    QueryParallelStrategyConfigHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, RankAndBubbleTimeHandlerWithStepIdError)
{
    auto request = std::make_unique<PipelineRankTimeRequest>();
    request->params.stepId = ";";
    request->params.stageId = "1";
    RankAndBubbleTimeHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, RankAndBubbleTimeHandlerWithStageIdError)
{
    auto request = std::make_unique<PipelineRankTimeRequest>();
    request->params.stepId = "1";
    request->params.stageId = ";";
    RankAndBubbleTimeHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, RankAndBubbleTimeHandlerWithExecuteSqlFail)
{
    auto request = std::make_unique<PipelineRankTimeRequest>();
    request->params.stepId = "1";
    request->params.stageId = "1";
    RankAndBubbleTimeHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, SetParallelStrategyConfigHandlerWithParamError)
{
    auto request = std::make_unique<SetParallelStrategyRequest>();
    request->config.tpSize = NUMBER_FIVE_HUNDRED;
    request->config.dpSize = NUMBER_TEN;
    request->config.cpSize = NUMBER_TEN;
    request->config.ppSize = NUMBER_TEN;
    request->config.epSize = NUMBER_TEN;
    SetParallelStrategyConfigHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, SetParallelStrategyConfigHandlerWithExecuteSqlError)
{
    auto request = std::make_unique<SetParallelStrategyRequest>();
    request->config.tpSize = NUMBER_TEN;
    request->config.dpSize = NUMBER_TEN;
    request->config.cpSize = NUMBER_TEN;
    request->config.ppSize = NUMBER_TEN;
    request->config.epSize = NUMBER_TEN;
    SetParallelStrategyConfigHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryParallelismArrangementHandlerShouldReturnFalseWithParamError)
{
    auto request = std::make_unique<QueryParallelismArrangementRequest>();
    request->params.config.tpSize = 2; // 2
    request->params.config.dpSize = 4; // 4
    request->params.config.cpSize = 2; // 2
    request->params.config.ppSize = 2; // 2
    request->params.config.epSize = 0; // 0
    QueryParallelismArrangementHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryParallelismArrangementHandlerShouldReturnFalseWithDataBaseError)
{
    auto request = std::make_unique<QueryParallelismArrangementRequest>();
    request->params.config.tpSize = 2; // 2
    request->params.config.dpSize = 4; // 4
    request->params.config.cpSize = 2; // 2
    request->params.config.ppSize = 2; // 2
    request->params.config.epSize = 1; // 1
    QueryParallelismArrangementHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryParallelismPerformanceHandlerShouldReturnFalseWithParamError)
{
    auto request = std::make_unique<QueryParallelismPerformanceRequest>();
    request->params.config.tpSize = 2; // 2
    request->params.config.dpSize = 4; // 4
    request->params.config.cpSize = 2; // 2
    request->params.config.ppSize = 2; // 2
    request->params.config.epSize = 0; // 0
    QueryParallelismPerformanceHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryParallelismPerformanceHandlerShouldReturnTrue)
{
    auto request = std::make_unique<QueryParallelismPerformanceRequest>();
    request->params.config.tpSize = 2; // 2
    request->params.config.dpSize = 4; // 4
    request->params.config.cpSize = 2; // 2
    request->params.config.ppSize = 2; // 2
    request->params.config.epSize = 1; // 1
    request->params.dimension = "ep-dp";
    QueryParallelismPerformanceHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(HandlerTest, StageAndBubbleTimeHandlerWithParamError)
{
    auto request = std::make_unique<PipelineStageTimeRequest>();
    request->params.stepId = ";";
    request->params.stageId = "1";
    StageAndBubbleTimeHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, StageAndBubbleTimeHandlerWithExecuteSqlFail)
{
    auto request = std::make_unique<PipelineStageTimeRequest>();
    request->params.stepId = "1";
    request->params.stageId = "1";
    StageAndBubbleTimeHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, StageHandlerWithParamError)
{
    auto request = std::make_unique<PipelineStageRequest>();
    request->params.stepId = ";";
    StageHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, StageHandlerWithExecuteSqlFail)
{
    auto request = std::make_unique<PipelineStageRequest>();
    request->params.stepId = "100";
    StageHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, StepHandlerWithExecuteSqlFail)
{
    auto request = std::make_unique<PipelineStepRequest>();
    StepHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, SummaryTopRankHandlerHandleRequestReturnTrue)
{
    auto request = std::make_unique<SummaryTopRankRequest>();
    SummaryTopRankHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, true);
}

TEST_F(HandlerTest, ImportExpertDataHandlerRequestReturnFalse)
{
    auto request = std::make_unique<ImportExpertDataRequest>();
    request->params.version = "1";
    request->params.filePath = "filePath";
    ImportExpertDataHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryExpertHotspotHandlerRequestLayerAbnormal)
{
    auto request = std::make_unique<QueryExpertHotspotRequest>();
    request->params.modelStage = "prefill";
    request->params.version = "1";
    request->params.denseLayerList = {0, 1};
    const int layerNum = -1;
    const int expertNum = 256;
    request->params.layerNum = layerNum;
    request->params.expertNum = expertNum;
    QueryExpertHotspotHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}

TEST_F(HandlerTest, QueryExpertHotspotHandlerRequestDenseAbnormal)
{
    auto request = std::make_unique<QueryExpertHotspotRequest>();
    request->params.modelStage = "prefill";
    request->params.version = "1";
    request->params.denseLayerList = {0, 61};
    const int layerNum = 60;
    const int expertNum = 256;
    request->params.layerNum = layerNum;
    request->params.expertNum = expertNum;
    QueryExpertHotspotHandler handler;
    bool result = handler.HandleRequest(std::move(request));
    EXPECT_EQ(result, false);
}
