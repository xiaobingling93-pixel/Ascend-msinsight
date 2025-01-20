/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ComputeQuerySourceApiHandlerTest.h"
#include "QueryApiInstructionsDynamicHandler.h"
#include "ComputeSourceFile.h"
#include "SourceProtocolRequest.h"
#include "ServerDefs.h"

using namespace Dic::Server;
using namespace Dic::Module;
using namespace Dic::Module::Source;
using namespace Dic::Module::Source::Test;

class QueryApiInstructionsDynamicHandlerTest {};

class HandlerDerived : public QueryApiInstructionsDynamicHandler {
public:
    void SetResponseBody(SourceApiInstrDynamicResponse &response,
             SourceApiInstrDynamicRequest &request)
    {
        QueryApiInstructionsDynamicHandler::SetResponseBody(response, request);
    }
};

TEST_F(ComputeQuerySourceApiHandlerTest, testQueryApiInstructionsDynamicHandlerRequestWithValidData)
{
    QueryApiInstructionsDynamicHandler handler;
    auto ptr = std::make_unique<SourceApiInstrDynamicRequest>();
    ptr->params.coreName = CORE_NAME;
    ptr->moduleName = "Source";
    ptr->projectName = "project";
    handler.HandleRequest(std::move(ptr));
}

TEST_F(ComputeQuerySourceApiHandlerTest,
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