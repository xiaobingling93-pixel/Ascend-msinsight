/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "document.h"
#include "ConstantDefs.h"
#include "JupyterProtocolUtil.h"
#include "JupyterProtocol.h"
#include "JupyterProtocolEvent.h"

using namespace Dic::Protocol;
class JupyterProtocolUtilTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        jupyterProtocol.Register();
    }

    void TearDown() override
    {
        jupyterProtocol.UnRegister();
    }

    Dic::Protocol::JupyterProtocol jupyterProtocol;
};

TEST_F(JupyterProtocolUtilTest, ToParseJupyterCompletedEventJsonTestReturnNormal)
{
    ParseJupyterCompletedEvent event{};
    std::string err;
    event.body.parseResult = Dic::PARSE_RESULT_OK;
    event.body.url = "http://localhost:8080";
    std::optional<Dic::document_t> jsonOptional = jupyterProtocol.ToJson(event, err);
    EXPECT_EQ(jsonOptional.has_value(), true);
    EXPECT_EQ(jsonOptional.value().HasMember("body"), true);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("parseResult"), true);
    EXPECT_EQ(jsonOptional.value()["body"].HasMember("url"), true);
    EXPECT_EQ(jsonOptional.value()["body"]["parseResult"].GetString(), event.body.parseResult);
    EXPECT_EQ(jsonOptional.value()["body"]["url"].GetString(), event.body.url);
}