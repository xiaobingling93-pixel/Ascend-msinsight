/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ComputeQuerySourceApiHandlerTest.h"
#include "QueryApiLineHandler.h"
#include "SourceProtocolRequest.h"
#include "QueryCodeFileHandler.h"

using namespace Dic::Server;
using namespace Dic::Module;
using namespace Dic::Module::Source;
using namespace Dic::Module::Source::Test;

TEST_F(ComputeQuerySourceApiHandlerTest, testQueryApiLineHandlerRequestWithValidData)
{
    QueryApiLineHandler handler;
    auto ptr = std::make_unique<SourceApiLineRequest>();
    ptr->params.sourceName = SOURCE_NAME;
    ptr->params.coreName = CORE_NAME;
    ptr->moduleName = "Source";
    ptr->projectName = "project";
    handler.HandleRequest(std::move(ptr));
}

TEST_F(ComputeQuerySourceApiHandlerTest, testQueryCodeFileHandlerRequestWithValidData)
{
    QueryCodeFileHandler handler;
    auto ptr = std::make_unique<SourceCodeFileRequest>();
    ptr->params.sourceName = SOURCE_NAME;
    handler.HandleRequest(std::move(ptr));
}