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

#ifndef PROFILER_SERVER_DETAILSCOMPUTELOADJSON_H
#define PROFILER_SERVER_DETAILSCOMPUTELOADJSON_H

#include <string>

const std::string DETAILS_BASE_INFO_JSON = R"(
{
	"advice" : [],
	"block_detail" : [{
			"block_id" : 0,
			"duration" : [6.648484706878662]
		}, {
			"block_id" : 1,
			"duration" : [7.250303268432617]
		}, {
			"block_id" : 2,
			"duration" : [7.144242286682129]
		}, {
			"block_id" : 3,
			"duration" : [7.40969705581665]
		}, {
			"block_id" : 4,
			"duration" : [7.361212253570557]
		}, {
			"block_id" : 5,
			"duration" : [7.270303249359131]
		}, {
			"block_id" : 6,
			"duration" : [5.176363468170166]
		}, {
			"block_id" : 7,
			"duration" : [3.9666666984558105]
		}
	],
	"block_dim" : 8,
	"duration" : 13.0600004196167,
	"mix_block_dim" : -1,
	"name" : "sin_custom",
	"op_type" : "vector",
	"soc" : "Ascend910B4"
}
)";

const std::string DETAILS_COMPUTE_LOAD_GRAPH_JSON = R"(
{
	"advice" : [],
	"subblock_detail" : [{
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 0.5630180835723877
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 0.5655055046081543
			}
		}, {
			"block_id" : 2,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 0.5502190589904785
			}
		}, {
			"block_id" : 3,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 0.551541268825531
			}
		}, {
			"block_id" : 4,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 0.5366011261940002
			}
		}, {
			"block_id" : 5,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 0.542549729347229
			}
		}, {
			"block_id" : 6,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 0.3684789836406708
			}
		}, {
			"block_id" : 7,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 0.37014925479888916
			}
		}
	]
}
)";

const std::string DETAILS_COMPUTE_LOAD_TABLE_JSON = R"(
{
	"advice" : [],
	"subblock_detail" : [{
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 3606.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "S32",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "FP32",
				"unit" : 1,
				"value" : 3584.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "FP16",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "S16",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "fusion",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "DB para",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL BLOCK",
				"unit" : 1,
				"value" : 2184.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "Bank group conflict BLOCK",
				"unit" : 1,
				"value" : 1892.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "Bank conflict BLOCK",
				"unit" : 1,
				"value" : 292.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "VALU resource conflict BLOCK",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "MTE urgent request BLOCK",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "WAIT",
				"unit" : 0,
				"value" : 1.4527273178100586
			}
		}, {
			"block_id" : 0,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "COMPUTE DATA SIZE",
				"unit" : 2,
				"value" : 229376.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL_ACTIVE",
				"unit" : 1,
				"value" : 3606.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "S32",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "FP32",
				"unit" : 1,
				"value" : 3584.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "FP16",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "S16",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "misc",
				"unit" : 1,
				"value" : 22.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "fusion",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "DB para",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "ALL BLOCK",
				"unit" : 1,
				"value" : 2184.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "Bank group conflict BLOCK",
				"unit" : 1,
				"value" : 1892.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "Bank conflict BLOCK",
				"unit" : 1,
				"value" : 292.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "VALU resource conflict BLOCK",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "MTE urgent request BLOCK",
				"unit" : 1,
				"value" : 0.0
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "WAIT",
				"unit" : 0,
				"value" : 1.421212077140808
			}
		}, {
			"block_id" : 1,
			"block_type" : "vector0",
			"data_detail" : {
				"name" : "COMPUTE DATA SIZE",
				"unit" : 2,
				"value" : 229376.0
			}
		}
	]
}
)";

#endif // PROFILER_SERVER_DETAILSCOMPUTELOADJSON_H
