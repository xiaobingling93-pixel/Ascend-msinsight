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

#include "InterCoreLoadGraphParser.h"
#include "JsonUtil.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Protocol;
using namespace Dic::Server;

bool InterCoreLoadGraphParser::GetInterCoreLoadAnalysisInfo(const std::string &json,
                                                            Protocol::DetailsInterCoreLoadGraphBody &body)
{
    std::optional<InterCoreLoadAnalysisDetail> analysisDetailOpt = ParseInterCoreLoadAnalysisInfo(json);
    if (!analysisDetailOpt.has_value()) {
        return false;
    }
    InterCoreLoadAnalysisDetail &analysisDetail = analysisDetailOpt.value();
    body.advice = analysisDetail.advice;
    body.opType = analysisDetail.opType;
    body.soc = analysisDetail.soc;
    for (const auto &opDetail: analysisDetail.opDetails) {
        DetailsInterCoreLoadOpDetail bodyOpDetail;
        bodyOpDetail.coreId = opDetail.coreId;
        for (const auto &subCoreDetail: opDetail.subCoreDetails) {
            DetailsInterCoreLoadSubCoreDetail bodySubCoreDetail;
            bodySubCoreDetail.SetCyclesDimension(subCoreDetail.cycles,
                                                 analysisDetail.minCycleMap[subCoreDetail.subCoreType]);
            bodySubCoreDetail.SetThroughputDimension(subCoreDetail.throughput,
                                                     analysisDetail.minThroughputMap[subCoreDetail.subCoreType]);
            bodySubCoreDetail.SetCacheHitRateDimension(subCoreDetail.hitRate,
                                                       analysisDetail.maxHitRateMap[subCoreDetail.subCoreType]);
            bodySubCoreDetail.SetSubCoreName(subCoreDetail.subCoreType, subCoreDetail.subCoreIndex);
            bodyOpDetail.AddSubCoreDetail(std::move(bodySubCoreDetail));
        }
        body.AddOpDetail(std::move(bodyOpDetail));
    }
    
    return true;
}

/*
{   // 核间负载分析数据块结构
  "soc": str,       // 算子运行平台
  "op_type": str,   // 算子类型：vector, cube, mix
  "advice": str,    // 分析结果
  "op_detail": [
    {
      "core_id": uint8,     // core序号
      "core_detail": [
        {
          "subcore_id": uint8,              // 0、1
          "subcore_type": str               // cube、vector
          "cycles": uint64,                 // 耗时（cycle）
          "throughput" : float32,           // 核吞吐数据量（GB/s）
          " L2cache _hit_rate" : float32    //  L2cache命中率  (%)
        }
      ]
    }
  ]
}
 */
std::optional<InterCoreLoadAnalysisDetail> InterCoreLoadGraphParser::ParseInterCoreLoadAnalysisInfo(
    const std::string &json)
{
    if (json.empty()) {
        ServerLog::Warn("Inter core load analysis json string is empty.");
        return std::nullopt;
    }
    // 解析json并统计每个维度指标的最优值
    InterCoreLoadAnalysisDetail analysisDetail;
    try {
        std::string errorStr;
        const std::optional<document_t> &jsonInfo = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(json, errorStr);
        if (!errorStr.empty() || !jsonInfo.has_value()) {
            ServerLog::Error("Try to parse inter core load analysis json string failed, error is ", errorStr);
            return std::nullopt;
        }
        const document_t &jsonInfoDoc = jsonInfo.value();
        analysisDetail.soc = JsonUtil::GetString(jsonInfoDoc, "soc");
        analysisDetail.opType = JsonUtil::GetString(jsonInfoDoc, "op_type");
        analysisDetail.advice = JsonUtil::GetString(jsonInfoDoc, "advice");

        // 解析op detail数组
        if (!JsonUtil::IsJsonArray(jsonInfoDoc, "op_detail")) {
            ServerLog::Warn("Try to parse inter core load analysis json failed cause op detail is not an array.");
            return std::nullopt;
        }
        const json_t &jsonOpDetailArray = jsonInfoDoc["op_detail"];
        ParseJsonOpDetailArray(analysisDetail, jsonOpDetailArray);
    } catch (const std::exception &e) {
        ServerLog::Error("Try to parse inter core load analysis json failed, exception is ", e.what());
        return std::nullopt;
    }
    
    return {std::move(analysisDetail)};
}

void InterCoreLoadGraphParser::ParseJsonOpDetailArray(InterCoreLoadAnalysisDetail &analysisDetail,
                                                      const json_t &jsonOpDetailArray)
{
    std::string_view errMsg;
    for (auto &jsonOpDetail: jsonOpDetailArray.GetArray()) {
        // 解析op detail
        InterCoreOpDetail opDetail;
        opDetail.coreId = JsonUtil::GetInteger(jsonOpDetail, "core_id");

        // 解析sub core detail数组
        if (!JsonUtil::IsJsonArray(jsonOpDetail, "core_detail")) {
            if (errMsg.empty()) {
                errMsg = "Found sub core detail is not array when parse inter core load analysis json.";
            }
            analysisDetail.AddOpDetail(std::move(opDetail));
            continue;
        }
        const json_t &jsonCoreDetailArray = jsonOpDetail["core_detail"];
        for (const auto &jsonCoreDetail: jsonCoreDetailArray.GetArray()) {
            // 解析sub core detail
            InterCoreSubCoreDetail subCoreDetail;
            subCoreDetail.subCoreIndex = JsonUtil::GetInteger(jsonCoreDetail, "subcore_id");
            subCoreDetail.subCoreType = JsonUtil::GetString(jsonCoreDetail, "subcore_type");
            int64_t tmp = JsonUtil::GetInteger(jsonCoreDetail, "cycles");
            subCoreDetail.cycles = tmp < 0 ? 0 :  static_cast<uint64_t>(tmp);
            tmp = JsonUtil::GetInteger(jsonCoreDetail, "throughput");
            subCoreDetail.throughput = tmp < 0 ? 0 :  static_cast<uint64_t>(tmp);
            subCoreDetail.hitRate = JsonUtil::GetFloat(jsonCoreDetail, "L2cache_hit_rate");

            // 设置最优值
            analysisDetail.SetMaxHitRate(subCoreDetail.subCoreType, subCoreDetail.hitRate);
            analysisDetail.SetMinThroughput(subCoreDetail.subCoreType, subCoreDetail.throughput);
            analysisDetail.SetMinCycle(subCoreDetail.subCoreType, subCoreDetail.cycles);
            opDetail.subCoreDetails.emplace_back(subCoreDetail);
        }
        analysisDetail.AddOpDetail(std::move(opDetail));
    }
    if (!errMsg.empty()) {
        ServerLog::Warn(errMsg);
    }
    TransformAnalysisDetail(analysisDetail);
}

void InterCoreLoadGraphParser::TransformAnalysisDetail(InterCoreLoadAnalysisDetail &analysisDetail)
{
    if (analysisDetail.opType != "vector") {
        return;
    }
    std::sort(analysisDetail.opDetails.begin(), analysisDetail.opDetails.end(),
              [](const InterCoreOpDetail &a, const InterCoreOpDetail &b) {
                  return a.coreId < b.coreId;
              });

    std::vector<InterCoreOpDetail> newOpDetails;
    uint8_t index = 0;
    while (index < std::numeric_limits<uint8_t>::max() - 2 && index < analysisDetail.opDetails.size()) {
        // 按照2个一组，将vector计算单元，分配到新的core op detail中
        uint8_t coreId = static_cast<uint8_t>(index / 2);
        InterCoreOpDetail detail;
        detail.coreId = coreId;
        Try2MoveSubCoreDetails(analysisDetail.opDetails[index], detail, SUB_CORE_INDEX_0);
        if (++index < analysisDetail.opDetails.size()) {
            Try2MoveSubCoreDetails(analysisDetail.opDetails[index], detail, SUB_CORE_INDEX_1);
        }
        newOpDetails.emplace_back(detail);
        index++;
    }
    analysisDetail.opDetails = std::move(newOpDetails);
}

void InterCoreLoadGraphParser::Try2MoveSubCoreDetails(InterCoreOpDetail &source, InterCoreOpDetail &dest,
    uint8_t subCoreIndex)
{
    if (!source.subCoreDetails.empty()) {
        auto subCoreDetail = source.subCoreDetails[0];
        subCoreDetail.subCoreIndex = subCoreIndex;
        dest.subCoreDetails.emplace_back(subCoreDetail);
    }
}

} // Dic
} // Module
} // Source