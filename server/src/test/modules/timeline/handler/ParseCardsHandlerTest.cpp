/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "HandlerTest.cpp"
#include "ParseCardsHandler.h"
#include "ParserStatusManager.h"

class ParseCardsHandlerTest : public HandlerTest {};

TEST_F(ParseCardsHandlerTest, ParseCardsHandlerTestCardFilePathIsEmpty)
{
    Dic::Module::Timeline::ParseCardsHandler parseCardsHandler;
    auto requestPtr = std::make_unique<Dic::Protocol::ParseCardsRequest>();
    requestPtr->params.cards = { "card1", "card2" };
    requestPtr->params.fileIds = { "fileId1", "fileId2" };
    Module::Timeline::ParserStatusManager::Instance().SetPendingStatus("card1", std::make_pair<ProjectTypeEnum, std::vector<std::string>>(ProjectTypeEnum::DB, {}));
    Module::Timeline::ParserStatusManager::Instance().SetPendingStatus("card2", std::make_pair<ProjectTypeEnum, std::vector<std::string>>(ProjectTypeEnum::TRACE, {"invalid filePath"}));
    EXPECT_EQ(parseCardsHandler.HandleRequest(std::move(requestPtr)), true);
}
