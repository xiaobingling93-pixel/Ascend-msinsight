/*
* Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

#include <gtest/gtest.h>
#include "InterCoreLoadGraphParser.h"
#include "SourceProtocolRequest.h"
#include "SourceFileParser.h"
#include "SourceProtocolUtil.h"
#include "WsSessionManager.h"
#include "InterCoreLoadTest.h"

namespace Dic::Module::Source::Test {
using namespace Dic::Module::Source;
using namespace Dic::Protocol;

class InterCoreLoadTest : public ::testing::Test {};

TEST_F(InterCoreLoadTest, test_GetInterCoreLoadAnalysisInfo_with_normal_json)
{
    InterCoreLoadGraphParser parser;
    DetailsInterCoreLoadGraphBody body;
    parser.GetInterCoreLoadAnalysisInfo(INTER_CORE_LOAD_ANALYSIS_JSON, body);
    // 校验解析的结果
    EXPECT_STREQ(body.soc.c_str(), "Ascend910B4");
    EXPECT_STREQ(body.opType.c_str(), "mix");
    EXPECT_STREQ(body.advice.c_str(), "\t0) vector core0 subcore1 took more time than other core.\n");
    int opDetailSize = 1;
    EXPECT_EQ(body.opDetails.size(), opDetailSize);
    DetailsInterCoreLoadOpDetail &opDetail = body.opDetails[0];
    int coreId = 0;
    EXPECT_EQ(opDetail.coreId, coreId);
    int coreDetailSize = 3;
    EXPECT_EQ(opDetail.subCoreDetails.size(), coreDetailSize);
    DetailsInterCoreLoadSubCoreDetail &subCoreDetail = opDetail.subCoreDetails[0];
    EXPECT_STREQ(subCoreDetail.subCoreName.c_str(), "cube0");
    uint64_t cycles = 135938;
    int cycleLevel = 10;
    EXPECT_EQ(subCoreDetail.cycles.value.compare, cycles);
    EXPECT_EQ(subCoreDetail.cycles.level, cycleLevel);
    float throughput = 256;
    int throughputLevel = 10;
    EXPECT_FLOAT_EQ(subCoreDetail.throughput.value.compare, throughput);
    EXPECT_EQ(subCoreDetail.throughput.level, throughputLevel);
    float hitRate = 63.994083f;
    int hitRateLevel = 10;
    EXPECT_FLOAT_EQ(subCoreDetail.cacheHitRate.value.compare, hitRate);
    EXPECT_EQ(subCoreDetail.cacheHitRate.level, hitRateLevel);
}

TEST_F(InterCoreLoadTest, test_response_to_json)
{
    DetailsInterCoreLoadGraphResponse response;
    response.body = {
            "soc",
            "optype",
            "advice"
    };
    DetailsInterCoreLoadOpDetail opDetail0 = {
        0,
        {
            {
                "cube0",
                {100, 8},
                {10, 9},
                {98.8, 10}
                },
            {
                "vector0",
                {99, 7},
                {9, 8},
                {97.8, 9}
                }
            }
    };

    response.body.opDetails.emplace_back(opDetail0);
    const std::optional<document_t> &docOpt = ToResponseJson<DetailsInterCoreLoadGraphResponse>(response);
    EXPECT_TRUE(docOpt.has_value());
    document_t &doc = const_cast<document_t &>(docOpt.value());
    EXPECT_TRUE(doc.HasMember("body"));

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::string docString = buffer.GetString();
    EXPECT_TRUE(StringUtil::StartWith(docString, INTER_CORE_LOAD_ANALYSIS_RESPONSE_JSON));
}
}