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

#ifndef PROFILER_SERVER_INTERCORELOADTEST_H
#define PROFILER_SERVER_INTERCORELOADTEST_H

#include <string>

namespace Dic::Module::Source::Test {
const std::string INTER_CORE_LOAD_ANALYSIS_OF_VECTOR_JSON = R"(
{
	"advice" : "\t1) core3 vector1 took more time than other vector cores.\n",
	"op_detail" : [{
			"core_detail" : [{
					"L2cache_hit_rate" : "75.700935",
					"cycles" : "7114",
					"subcore_id" : "0",
					"subcore_type" : "vector",
					"throughput" : "15104"
				}
			],
			"core_id" : 0
		}, {
			"core_detail" : [{
					"L2cache_hit_rate" : "76.635513",
					"cycles" : "7453",
					"subcore_id" : "0",
					"subcore_type" : "vector",
					"throughput" : "15104"
				}
			],
			"core_id" : 1
		}, {
			"core_detail" : [{
					"L2cache_hit_rate" : "77.272728",
					"cycles" : "7940",
					"subcore_id" : "0",
					"subcore_type" : "vector",
					"throughput" : "15104"
				}
			],
			"core_id" : 2
		}
	],
	"op_type" : "vector",
	"soc" : "Ascend910B4"
}
)";
const std::string INTER_CORE_LOAD_ANALYSIS_JSON = R"(
{
    "advice": "\t0) vector core0 subcore1 took more time than other core.\n",
    "op_detail": [
        {
            "core_detail": [
                {
                    "L2cache_hit_rate": "63.994083",
                    "cycles": "135938",
                    "subcore_id": "0",
                    "subcore_type": "cube",
                    "throughput": "256"
                },
                {
                    "L2cache_hit_rate": "87.804878",
                    "cycles": "76949",
                    "subcore_id": "0",
                    "subcore_type": "vector",
                    "throughput": "512"
                },
                {
                    "L2cache_hit_rate": "92.682930",
                    "cycles": "130426",
                    "subcore_id": "1",
                    "subcore_type": "vector",
                    "throughput": "128"
                }
            ],
            "core_id": 0
        }
    ],
    "op_type": "mix",
    "soc": "Ascend910B4"
}
)";

const std::string INTER_CORE_LOAD_ANALYSIS_RESPONSE_JSON =
    R"({"type":"response","id":0,"requestId":0,"result":false,"command":"source/details/interCoreLoadAnalysis",)"
    R"("moduleName":"unknown","body":{"soc":"soc","opType":"optype","advice":"advice",)"
    R"("opDetails":[{"coreId":0,"subCoreDetails":[{"subCoreName":"cube0",)";

// 测试场景：包含 simt_vf_instructions 字段的 JSON 数据
// 注意：使用可被二进制精确表示的浮点数（如 2.5、1.25、2.0）避免精度问题
const std::string INTER_CORE_LOAD_ANALYSIS_WITH_SIMT_JSON = R"(
{
    "advice": "\t0) Test advice with simt instructions.\n",
    "op_detail": [
        {
            "core_detail": [
                {
                    "L2cache_hit_rate": "85.5",
                    "cycles": "10000",
                    "subcore_id": "0",
                    "subcore_type": "vector",
                    "throughput": "512",
                    "simt_vf_instructions": {
                        "instructions": "25600",
                        "instructions_per_cycle": "2.5"
                    }
                },
                {
                    "L2cache_hit_rate": "90.25",
                    "cycles": "12000",
                    "subcore_id": "1",
                    "subcore_type": "vector",
                    "throughput": "256",
                    "simt_vf_instructions": {
                        "instructions": "19200",
                        "instructions_per_cycle": "1.25"
                    }
                }
            ],
            "core_id": 0
        },
        {
            "core_detail": [
                {
                    "L2cache_hit_rate": "88.0",
                    "cycles": "11000",
                    "subcore_id": "0",
                    "subcore_type": "vector",
                    "throughput": "400",
                    "simt_vf_instructions": {
                        "instructions": "22000",
                        "instructions_per_cycle": "2.0"
                    }
                }
            ],
            "core_id": 1
        }
    ],
    "op_type": "vector",
    "soc": "Ascend910B4"
}
)";

// 测试场景：不包含 simt_vf_instructions 字段的 JSON 数据（向后兼容）
const std::string INTER_CORE_LOAD_ANALYSIS_WITHOUT_SIMT_JSON = R"(
{
    "advice": "\t0) Test advice without simt instructions.\n",
    "op_detail": [
        {
            "core_detail": [
                {
                    "L2cache_hit_rate": "75.0",
                    "cycles": "8000",
                    "subcore_id": "0",
                    "subcore_type": "cube",
                    "throughput": "256"
                }
            ],
            "core_id": 0
        }
    ],
    "op_type": "cube",
    "soc": "Ascend910B4"
}
)";

}
#endif // PROFILER_SERVER_INTERCORELOADTEST_H
