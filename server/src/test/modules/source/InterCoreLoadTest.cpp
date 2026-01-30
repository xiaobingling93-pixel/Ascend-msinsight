/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include <gtest/gtest.h>
#include "InterCoreLoadGraphParser.h"
#include "SourceProtocolRequest.h"
#include "SourceFileParser.h"
#include "SourceProtocolUtil.h"
#include "WsSessionManager.h"
#include "InterCoreLoadTest.h"
#include "JsonUtil.h"

namespace Dic::Module::Source::Test {
using namespace Dic::Module::Source;
using namespace Dic::Protocol;

class InterCoreLoadTest : public ::testing::Test {
protected:
    document_t GeneratorBigCoreDetail(size_t len)
    {
        document_t doc(kObjectType);
        auto &allocator = doc.GetAllocator();
        JsonUtil::AddMember(doc, "soc", "910B", allocator);
        JsonUtil::AddMember(doc, "op_type", "vector", allocator);
        JsonUtil::AddMember(doc, "advice", "test advice", allocator);
        json_t op_detail(kArrayType);
        for (size_t i = 0; i < len; i++) {
            json_t detail(kObjectType);
            JsonUtil::AddMember(detail, "core_id", std::to_string(i), allocator);
            json_t core_detail(kArrayType);
            JsonUtil::AddMember(detail, "core_detail", core_detail, allocator);
            op_detail.PushBack(detail, allocator);
        }
        JsonUtil::AddMember(doc, "op_detail", op_detail, allocator);
        return doc;
    }
};

TEST_F(InterCoreLoadTest, test_GetInterCoreLoadAnalysisInfo_with_normal_json_of_vector_op_type)
{
    InterCoreLoadGraphParser parser;
    DetailsInterCoreLoadGraphBody body;
    parser.GetInterCoreLoadAnalysisInfo(INTER_CORE_LOAD_ANALYSIS_OF_VECTOR_JSON, body);
    // 校验解析的结果
    EXPECT_STREQ(body.soc.c_str(), "Ascend910B4");
    EXPECT_STREQ(body.opType.c_str(), "vector");
    EXPECT_STREQ(body.advice.c_str(), "\t1) core3 vector1 took more time than other vector cores.\n");
    int opDetailSize = 2;
    EXPECT_EQ(body.opDetails.size(), opDetailSize);
    DetailsInterCoreLoadOpDetail &opDetail = body.opDetails[0];
    int coreId = 0;
    EXPECT_EQ(opDetail.coreId, coreId);
    int coreDetailSize = 2;
    EXPECT_EQ(opDetail.subCoreDetails.size(), coreDetailSize);
    DetailsInterCoreLoadSubCoreDetail &subCoreDetail = opDetail.subCoreDetails[0];
    EXPECT_STREQ(subCoreDetail.subCoreName.c_str(), "vector0");
    uint64_t cycles = 7114;
    int cycleLevel = 2;
    EXPECT_EQ(subCoreDetail.cycles.value.compare, cycles);
    EXPECT_EQ(subCoreDetail.cycles.level, cycleLevel);
    float throughput = 15104;
    int throughputLevel = 0;
    EXPECT_FLOAT_EQ(subCoreDetail.throughput.value.compare, throughput);
    EXPECT_EQ(subCoreDetail.throughput.level, throughputLevel);
    float hitRate = 75.700935;
    int hitRateLevel = 2;
    EXPECT_FLOAT_EQ(subCoreDetail.cacheHitRate.value.compare, hitRate);
    EXPECT_EQ(subCoreDetail.cacheHitRate.level, hitRateLevel);
}

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
    int cycleLevel = 0;
    EXPECT_EQ(subCoreDetail.cycles.value.compare, cycles);
    EXPECT_EQ(subCoreDetail.cycles.level, cycleLevel);
    float throughput = 256;
    int throughputLevel = 0;
    EXPECT_FLOAT_EQ(subCoreDetail.throughput.value.compare, throughput);
    EXPECT_EQ(subCoreDetail.throughput.level, throughputLevel);
    float hitRate = 63.994083f;
    int hitRateLevel = 0;
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

TEST_F(InterCoreLoadTest, test_op_detail_array_size_bigger_than_uint8_max)
{
    InterCoreLoadGraphParser parser;
    DetailsInterCoreLoadGraphBody body;
    auto json = GeneratorBigCoreDetail(static_cast<size_t>(std::numeric_limits<uint8_t>::max()) * 2);
    std::string jsonStr = JsonUtil::JsonDump(json);
    EXPECT_NO_THROW(parser.GetInterCoreLoadAnalysisInfo(jsonStr, body));
}
}