/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SourceModule.h"
#include "ProtocolDefs.h"
#include "SourceProtocolRequest.h"

using namespace Dic::Module;

class ComputeSourceModuleTest : public ::testing::Test {
};

class SourceModuleTemp : public SourceModule {
public:
    std::string GetModuleName()
    {
        return moduleName;
    }

    std::map<std::string, std::unique_ptr<ModuleRequestHandler>>& GetRegister()
    {
        return requestHandlerMap;
    }
};

TEST_F(ComputeSourceModuleTest, ConstructorAndDestructorTest)
{
    SourceModuleTemp module;
    EXPECT_EQ(Dic::Protocol::MODULE_SOURCE, module.GetModuleName());
}

TEST_F(ComputeSourceModuleTest, RegisterRequestHandlersTest)
{
    SourceModuleTemp module;
    module.RegisterRequestHandlers();
    EXPECT_TRUE(!module.GetRegister().empty());
    EXPECT_TRUE(module.GetRegister().find(Dic::Protocol::REQ_RES_SOURCE_CODE_FILE) != module.GetRegister().end());
}

TEST_F(ComputeSourceModuleTest, OnRequestTest)
{
    SourceModuleTemp module;
    auto requestPtr = std::make_unique<Dic::Protocol::SourceApiInstrRequest>();
    module.OnRequest(std::move(requestPtr));
}