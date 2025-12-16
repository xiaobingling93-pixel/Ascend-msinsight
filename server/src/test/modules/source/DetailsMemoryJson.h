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

#ifndef PROFILER_SERVER_DETAILSMEMORYJSON_H
#define PROFILER_SERVER_DETAILSMEMORYJSON_H

#include <string>

const std::string DETAILS_MEMORY_GRAPH_JSON = R"(
{
	"core_memory_map" : [{
			"Cube" : {
				"cycle" : 80,
				"ratio" : 0.020524030551314354,
				"total_cycles" : 389787
			},
			"Vector" : {
				"cycle" : 65964,
				"ratio" : 16.9230899810791,
				"total_cycles" : 389787
			},
			"Vector1" : {
				"cycle" : 65964,
				"ratio" : 16.9230899810791,
				"total_cycles" : 389787
			},
			"L2cache" : {
				"hit" : 13,
				"hit_ratio" : 0.16883116960525513,
				"miss" : 64,
				"total_request" : 77
			},
			"advice" : [],
			"core_no" : 0,
			"memory_unit" : [{
					"bandwidth" : 0.03278358653187752,
					"display" : 1,
					"memory_path" : 12,
					"peak_ratio" : 0.042693182826042175,
					"request" : 257
				}, {
					"bandwidth" : 0.032656021416187286,
					"display" : 1,
					"memory_path" : 13,
					"peak_ratio" : 0.05638418719172478,
					"request" : 256
				}, {
					"bandwidth" : 0.17563675343990326,
					"display" : 1,
					"memory_path" : 14,
					"peak_ratio" : 0.0,
					"request" : 3584
				}, {
					"bandwidth" : 0.25718238949775696,
					"display" : 1,
					"memory_path" : 15,
					"peak_ratio" : 0.0,
					"request" : 5248
				}
			],
			"op_type" : "vector",
			"soc" : "910B"
		}, {
			"L2cache" : {
				"hit" : 13,
				"hit_ratio" : 0.16883116960525513,
				"miss" : 64,
				"total_request" : 77
			},
			"advice" : [],
			"core_no" : 1,
			"memory_unit" : [{
					"bandwidth" : 0.03295082971453667,
					"display" : 1,
					"memory_path" : 12,
					"peak_ratio" : 0.0429109781985145,
					"request" : 257
				}, {
					"bandwidth" : 0.0328226163983345,
					"display" : 1,
					"memory_path" : 13,
					"peak_ratio" : 0.056671835482120514,
					"request" : 256
				}, {
					"bandwidth" : 0.17653276026248932,
					"display" : 1,
					"memory_path" : 14,
					"peak_ratio" : 0.0,
					"request" : 3584
				}, {
					"bandwidth" : 0.25849440693855286,
					"display" : 1,
					"memory_path" : 15,
					"peak_ratio" : 0.0,
					"request" : 5248
				}
			],
			"op_type" : "vector",
			"soc" : "910B"
		}
	]
}
)";

const std::string DETAILS_MEMORY_TABLE_JSON = R"JSON(
{
	"table_per_block" : [{
			"advice" : [],
			"block_id" : 0,
			"table_detail" : [{
					"header_name" : ["", "hit", "miss", "total", "hit rate"],
					"row" : [{
							"name" : "L2 Cache Write",
							"value" : [13, 64, 77, 0.16883116960525513]
						}, {
							"name" : "L2 Cache Read",
							"value" : [16, 257, 273, 0.05860805884003639]
						}, {
							"name" : "L2 Cache Total",
							"value" : [29, 321, 350, 0.08285713940858841]
						}, {
							"name" : "iCache total",
							"value" : [379, 33, 412, 0.9199029207229614]
						}
					],
					"size" : [4, 4],
					"table_name" : "Cache"
				}, {
					"header_name" : ["", "requests", "throughput"],
					"row" : [{
							"name" : "Read Main Memory",
							"value" : [80, 0.0009801158448681235]
						}, {
							"name" : "Write Main Memory",
							"value" : [77, 0.0009433614904992282]
						}
					],
					"size" : [2, 2],
					"table_name" : "HBM"
				}, {
					"header_name" : ["", "instructions", "cycle", "throughput", "active rate(%)"],
					"row" : [{
							"name" : "MTE1",
							"value" : [0, 0, 0.0, 0, 0.0]
						}, {
							"name" : "MTE2",
							"value" : [4, 977, 0.0, 6138, 0.15917237102985382]
						}, {
							"name" : "MTE3",
							"value" : [3, 689, 0.0, 6959, 0.0990084782242775]
						}, {
							"name" : "FIXP",
							"value" : [0, 0, 0.0, 0, 0.0]
						}, {
							"name" : "Scalar",
							"value" : [0, 3278, 0.0, 0, 0.0]
						}
					],
					"size" : [5, 5],
					"table_name" : "Pipe"
				}, {
					"header_name" : ["", "requests", "throughput", "peak(%)"],
					"row" : [{
							"name" : "HBM Read",
							"value" : [257, 0.03278358653187752, 0.042693182826042175]
						}, {
							"name" : "HBM Write",
							"value" : [256, 0.032656021416187286, 0.05638418719172478]
						}, {
							"name" : "VEC Write",
							"value" : [257, 0.03278358653187752, 0.042693182826042175]
						}, {
							"name" : "Vec Read",
							"value" : [256, 0.032656021416187286, 0.05638418719172478]
						}, {
							"name" : "Scalar Write",
							"value" : [5248, 0.25718238949775696, 0.0]
						}, {
							"name" : "Scalar Read",
							"value" : [3584, 0.17563675343990326, 0.0]
						}
					],
					"size" : [6, 3],
					"table_name" : "UB"
				}, {
					"header_name" : ["", "requests", "throughput", "peak(%)"],
					"row" : [{
							"name" : "UB Write",
							"value" : [3584, 0.17563675343990326, 0.0]
						}, {
							"name" : "UB Read",
							"value" : [5248, 0.25718238949775696, 0.0]
						}
					],
					"size" : [2, 3],
					"table_name" : "VEC"
				}, {
					"header_name" : ["", "hit", "miss", "total", "hit rate"],
					"row" : [{
							"name" : "Write",
							"value" : [13, 64, 77, 0.16883116960525513]
						}, {
							"name" : "Read",
							"value" : [16, 257, 273, 0.05860805884003639]
						}, {
							"name" : "Total",
							"value" : [29, 321, 350, 0.08285713940858841]
						}
					],
					"size" : [3, 4],
					"table_name" : "iCache"
				}
			],
			"table_op_type" : "vector"
		}, {
			"advice" : [],
			"block_id" : 31,
			"table_detail" : [{
					"header_name" : ["", "hit", "miss", "total", "hit rate"],
					"row" : [{
							"name" : "L2 Cache Write",
							"value" : [13, 32, 45, 0.2888889014720917]
						}, {
							"name" : "L2 Cache Read",
							"value" : [16, 128, 144, 0.1111111119389534]
						}, {
							"name" : "L2 Cache Total",
							"value" : [29, 160, 189, 0.15343914926052094]
						}, {
							"name" : "iCache total",
							"value" : [244, 0, 244, 1.0]
						}
					],
					"size" : [4, 4],
					"table_name" : "Cache"
				}, {
					"header_name" : ["", "requests", "throughput"],
					"row" : [{
							"name" : "Read Main Memory",
							"value" : [48, 0.09904095530509949]
						}, {
							"name" : "Write Main Memory",
							"value" : [45, 0.09285089373588562]
						}
					],
					"size" : [2, 2],
					"table_name" : "HBM"
				}, {
					"header_name" : ["", "instructions", "cycle", "throughput", "active rate(%)"],
					"row" : [{
							"name" : "MTE1",
							"value" : [0, 0, 0.0, 0, 0.0]
						}, {
							"name" : "MTE2",
							"value" : [3, 688, 0.0, 3463, 0.19867166876792908]
						}, {
							"name" : "MTE3",
							"value" : [2, 256, 0.0, 3519, 0.07274793833494186]
						}, {
							"name" : "FIXP",
							"value" : [0, 0, 0.0, 0, 0.0]
						}, {
							"name" : "Scalar",
							"value" : [0, 1938, 0.0, 0, 0.0]
						}
					],
					"size" : [5, 5],
					"table_name" : "Pipe"
				}, {
					"header_name" : ["", "requests", "throughput", "peak(%)"],
					"row" : [{
							"name" : "HBM Read",
							"value" : [129, 2.771399736404419, 3.609119415283203]
						}, {
							"name" : "HBM Write",
							"value" : [128, 2.7499160766601563, 4.748030185699463]
						}, {
							"name" : "VEC Write",
							"value" : [129, 2.771399736404419, 3.609119415283203]
						}, {
							"name" : "Vec Read",
							"value" : [128, 2.7499160766601563, 4.748030185699463]
						}, {
							"name" : "Scalar Write",
							"value" : [2624, 21.656953811645508, 0.0]
						}, {
							"name" : "Scalar Read",
							"value" : [1792, 14.790115356445313, 0.0]
						}
					],
					"size" : [6, 3],
					"table_name" : "UB"
				}, {
					"header_name" : ["", "requests", "throughput", "peak(%)"],
					"row" : [{
							"name" : "UB Write",
							"value" : [1792, 14.790115356445313, 0.0]
						}, {
							"name" : "UB Read",
							"value" : [2624, 21.656953811645508, 0.0]
						}
					],
					"size" : [2, 3],
					"table_name" : "VEC"
				}, {
					"header_name" : ["", "hit", "miss", "total", "hit rate"],
					"row" : [{
							"name" : "Write",
							"value" : [13, 32, 45, 0.2888889014720917]
						}, {
							"name" : "Read",
							"value" : [16, 128, 144, 0.1111111119389534]
						}, {
							"name" : "Total",
							"value" : [29, 160, 189, 0.15343914926052094]
						}
					],
					"size" : [3, 4],
					"table_name" : "iCache"
				}
			],
			"table_op_type" : "vector"
		}
	]
}
)JSON";

#endif // PROFILER_SERVER_DETAILSMEMORYJSON_H
