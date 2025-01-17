/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "QueryApiInstructionsDynamicHandler.h"
#include "QueryApiLineHandlerTest.h"
#include "SourceProtocolRequest.h"
#include "../mockUtils/BinFileGenerator.h"
#include "WsSessionManager.h"
#include "ServerDefs.h"
#include "WsSessionImpl.h"

using namespace Dic::Server;
using namespace Dic::Module;
using namespace Dic::Module::Source;
using namespace Dic::Module::Source::Test;

class QueryApiInstructionsDynamicHandlerTest {};

class ComputeQueryApiInstructionsDynamicHandlerTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        // generate bin file
        auto apiPtr = std::make_unique<NormalDataBlock>(DataTypeEnum::API_FILE, std::string(API_FILE));
        auto instrPtr = std::make_unique<NormalDataBlock>(DataTypeEnum::API_INSTR, std::string(INSTR_FILE));
        auto sourcePtr = std::make_unique<SourceDataBlock>(std::string(SOURCE_FILE), std::string(SOURCE_NAME));
        BinFileGenerator generator;

        generator.AddDataBlock(std::move(apiPtr));
        generator.AddDataBlock(std::move(instrPtr));
        generator.AddDataBlock(std::move(sourcePtr));
        generator.Generate(std::string(BIN_FILE_PATH_API_LINE));

        // init parser
        std::string binPath = std::string(BIN_FILE_PATH_API_LINE);
        SourceFileParser::Instance().SetFilePath(binPath);
        SourceFileParser::Instance().Parse({binPath}, binPath, binPath);

        // init ws session
        WsChannel *ws;
        std::unique_ptr<WsSessionImpl> session = std::make_unique<WsSessionImpl>(ws);
        WsSessionManager::Instance().AddSession(std::move(session));
    }

    static void TearDownTestSuite()
    {
        // remove bin file
        Dic::Module::Source::Test::BinFileGenerator::RemoveFile(std::string(BIN_FILE_PATH_API_LINE));

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

class HandlerDerived : public QueryApiInstructionsDynamicHandler {
public:
    void SetResponseBody(SourceApiInstrDynamicResponse &response,
             SourceApiInstrDynamicRequest &request)
    {
        QueryApiInstructionsDynamicHandler::SetResponseBody(response, request);
    }
};

TEST_F(ComputeQueryApiInstructionsDynamicHandlerTest, testQueryApiInstructionsDynamicHandlerRequestWithValidData)
{
    QueryApiInstructionsDynamicHandler handler;
    auto ptr = std::make_unique<SourceApiInstrDynamicRequest>();
    ptr->params.coreName = CORE_NAME;
    ptr->moduleName = "Source";
    ptr->projectName = "project";
    handler.HandleRequest(std::move(ptr));
}

TEST_F(ComputeQueryApiInstructionsDynamicHandlerTest,
    testQueryApiInstructionsDynamicHandlerSetResponseBodyWithValidData)
{
    HandlerDerived handler;
    SourceApiInstrDynamicRequest request;
    request.params.coreName = CORE_NAME;
    SourceApiInstrDynamicResponse response;
    handler.SetResponseBody(response, request);
    EXPECT_EQ(response.body.coreName, CORE_NAME);
    auto colMap = response.body.columnNameMap;
    EXPECT_EQ(colMap["Address"], ColumDataType::Type::STRING);
    EXPECT_EQ(colMap["AscendC Inner Code"], ColumDataType::Type::STRING);
    EXPECT_EQ(colMap["Cycles"], ColumDataType::Type::INT);
    EXPECT_EQ(colMap["Instructions Executed"], ColumDataType::Type::INT);
    EXPECT_EQ(colMap["Pipe"], ColumDataType::Type::STRING);
    EXPECT_EQ(colMap["TheoreticalStallCycles"], ColumDataType::Type::INT);
    EXPECT_EQ(colMap["Source"], ColumDataType::Type::STRING);
    EXPECT_EQ(colMap["RealStallCycles"], ColumDataType::Type::INT);

    auto dataList = response.body.columnValues;
    EXPECT_TRUE(!dataList.empty());
    auto data  = dataList[0];
    EXPECT_EQ(data.stringMap["Address"], "0x1134e2d8");
    EXPECT_EQ(data.stringMap["AscendC Inner Code"], "/test/vec_add1_simt.cpp:50");
    EXPECT_EQ(data.intMap["Cycles"], 62); // Cycles is 62
    EXPECT_EQ(data.intMap["Instructions Executed"], 4); // Instructions Executed is 4
    EXPECT_EQ(data.stringMap["Pipe"], "RVECLD");
    EXPECT_EQ(data.intMap["TheoreticalStallCycles"], 8); // TheoreticalStallCycles is 8
    EXPECT_EQ(data.stringMap["Source"],
              "SIMT_LDG [PEX:6|P],[Rn:0|R],[Rn1:1|R],[Rd:0|R],[#ofst:9],[cop:1],[l2_cache_hint:0]");
    EXPECT_EQ(data.intMap["RealStallCycles"], 13); // RealStallCycles is 13
}