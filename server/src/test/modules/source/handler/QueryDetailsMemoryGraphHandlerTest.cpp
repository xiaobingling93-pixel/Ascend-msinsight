/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "../mockUtils/BinFileGenerator.h"
#include "WsSessionManager.h"
#include "ServerDefs.h"
#include "WsSessionImpl.h"
#include "../DetailsMemoryJson.h"
#include "QueryDetailsMemoryGraphHandler.h"
#include "QueryDetailsMemoryTableHandler.h"
#include "SourceProtocolRequest.h"

using namespace Dic::Server;
using namespace Dic::Module;
using namespace Dic::Module::Source;
using namespace Dic::Module::Source::Test;

static const std::string BIN_FILE_PATH = "query_details_memory_graph_test.bin";
class QueryDetailsMemoryGraphHandlerTest {};
class ComputeQueryDetailsMemoryGraphHandlerTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        // generate bin file
        auto graph = std::make_unique<NormalDataBlock>(DataTypeEnum::DETAILS_MEMORY_GRAPH, DETAILS_MEMORY_GRAPH_JSON);
        auto table = std::make_unique<NormalDataBlock>(DataTypeEnum::DETAILS_MEMORY_TABLE, DETAILS_MEMORY_TABLE_JSON);
        BinFileGenerator generator;

        generator.AddDataBlock(std::move(graph));
        generator.AddDataBlock(std::move(table));
        generator.Generate(BIN_FILE_PATH);

        // init parser
        SourceFileParser::Instance().SetFilePath(BIN_FILE_PATH);
        SourceFileParser::Instance().Parse({BIN_FILE_PATH}, BIN_FILE_PATH, BIN_FILE_PATH);

        // init ws session
        WsChannel *ws;
        std::unique_ptr<WsSessionImpl> session = std::make_unique<WsSessionImpl>(ws);
        WsSessionManager::Instance().AddSession(std::move(session));
    }

    static void TearDownTestSuite()
    {
        // remove bin file
        Dic::Module::Source::Test::BinFileGenerator::RemoveFile(BIN_FILE_PATH);

        // reset parser
        SourceFileParser::Instance().Reset();

        // remove ws session
        auto session = WsSessionManager::Instance().GetSession();
        if (session != nullptr) {
            session->SetStatus(WsSession::Status::CLOSED);
            session->WaitForExit();
            WsSessionManager::Instance().RemoveSession();
        }
    }
};

TEST_F(ComputeQueryDetailsMemoryGraphHandlerTest, testQueryDetailsMemoryGraphHandlerWithValidData)
{
    QueryDetailsMemoryGraphHandler handler;
    auto ptr = std::make_unique<DetailsMemoryGraphRequest>();
    ptr->params.blockId = "0";
    handler.HandleRequest(std::move(ptr));
}

TEST_F(ComputeQueryDetailsMemoryGraphHandlerTest, testQueryDetailsMemoryTableHandlerWithValidData)
{
    QueryDetailsMemoryTableHandler handler;
    auto ptr = std::make_unique<DetailsMemoryTableRequest>();
    ptr->params.blockId = "0";
    handler.HandleRequest(std::move(ptr));
}