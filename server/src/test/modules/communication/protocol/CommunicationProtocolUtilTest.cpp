/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include <GlobalDefs.h>
#include "ProtocolDefs.h"
#include "CommunicationProtocol.h"
#include "CommunicationProtocolResponse.h"

using namespace Dic::Protocol;
const uint64_t NUMBER_HUNDRED = 100;
const uint64_t NUMBER_THOUSAND = 100;
class CommunicationProtocolUtilTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        protocol.Register();
    }

    void TearDown() override
    {
        protocol.UnRegister();
    }

    Dic::Protocol::CommunicationProtocol protocol;
};

TEST_F(CommunicationProtocolUtilTest, ToOperatorListResponseTest)
{
    const std::string KEY_MIN_TIME = "minTime";
    const std::string KEY_MAX_TIME = "maxTime";
    Dic::Protocol::OperatorListsResponse response;
    std::string err;
    OperatorListsResponseBody body;
    response.body.rankLists.emplace_back("1");
    response.body.minTime = NUMBER_HUNDRED;
    response.body.maxTime = NUMBER_THOUSAND;
    OperatorTimeItem op = {"op1", 100, 900};
    CompareData<std::vector<OperatorTimeItem>> compareData;
    compareData.compare.push_back(op);
    response.body.opLists.push_back(compareData);
    std::optional<Dic::document_t> jsonOptional = protocol.ToJson(response, err);
    EXPECT_EQ(jsonOptional.value()["body"][KEY_MIN_TIME.c_str()], response.body.minTime);
    EXPECT_EQ(jsonOptional.value()["body"][KEY_MAX_TIME.c_str()], response.body.maxTime);
}