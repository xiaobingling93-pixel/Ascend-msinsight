/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "../mockUtils/BinFileGenerator.h"
#include "WsSessionManager.h"
#include "ServerDefs.h"
#include "WsSessionImpl.h"
#include "SourceProtocolRequest.h"
#include "QueryCachelineRecordHandler.h"

using namespace Dic::Server;
using namespace Dic::Module;
using namespace Dic::Module::Source;
using namespace Dic::Module::Source::Test;
const std::string DETAILS_CACHE_JSON = R"JSON({
    "Cacheline Records" : [
        {
            "Cacheline Id" : 138,
            "Hit" : {
                "Address Range" : [],
                "Value" : [0, 0.0]
            },
            "Miss" : {
                "Address Range" : [],
                "Value" : [0, 0.0]
            }
        },
        {
            "Cacheline Id" : 138,
            "Hit" : {
                "Address Range" : [],
                "Value" : [0, 0.0]
            },
            "Miss" : {
                "Address Range" : [],
                "Value" : [0, 0.0]
            }
        }
    ]
})JSON";

static const std::string BIN_FILE_PATH = "visualize_data_nosource.bin";

class QueryCachelineRecordHandlerTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        auto cache = std::make_unique<NormalDataBlock>(DataTypeEnum::DISPLAY_CACHE, DETAILS_CACHE_JSON);
        BinFileGenerator generator;
        generator.AddDataBlock(std::move(cache));
        generator.Generate(BIN_FILE_PATH);

        // init parser
        SourceFileParser::Instance().SetFilePath(BIN_FILE_PATH);
        SourceFileParser::Instance().Parse({BIN_FILE_PATH}, BIN_FILE_PATH, BIN_FILE_PATH, "");

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

TEST_F(QueryCachelineRecordHandlerTest, QueryCachelineRecordHandlerWithValidData)
{
    QueryCachelineRecordHandler handler;
    auto ptr = std::make_unique<DetailsMemoryGraphRequest>();
    EXPECT_TRUE(handler.HandleRequest(std::move(ptr)));
}
