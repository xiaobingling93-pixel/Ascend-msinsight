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

#ifndef PROFILER_SERVER_DETAILSROOFLINEJSON_H
#define PROFILER_SERVER_DETAILSROOFLINEJSON_H

#include <string>

const std::string ROOFLINE_DATA_BASE_INFO_JSON = R"JSON(
{
	"advice" : [],
	"block_dim" : "4",
	"device_id" : "0",
	"duration" : 301.0400085449219,
	"mix_block_detail" : {
		"head_name" : ["Block ID", "Cube0 Duration (μs)", "Vector0 Duration (μs)", "Vector1 Duration (μs)"],
		"row" : [{
				"value" : ["0", "165.255157", "160.726059", "165.080612"]
			}, {
				"value" : ["1", "167.384247", "162.390915", "167.221207"]
			}, {
				"value" : ["2", "160.872116", "160.895752", "156.722427"]
			}, {
				"value" : ["3", "159.159393", "159.349091", "154.552734"]
			}
		],
		"size" : [5, 4]
	},
	"mix_block_dim" : "8",
	"name" : "gen_FlashAttentionScore_10000000012201130953_mix_aic",
	"op_type" : "mix",
	"pid" : "3431487",
	"soc" : "Ascend910B4"
}
)JSON";

const std::string ROOFLINE_DATA_JSON = R"JSON(
{
	"advice" : "latency bound:pipeline caused",
	"multiple_rooflines" : [{
			"rooflines" : [{
					"bw" : 18.440187454223633,
					"bw_name" : "L1 Read + Write",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [33.28504180908203, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 4.610046863555908,
					"bw_name" : "Read from L0C",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [80.51353454589844, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 9.220093727111816,
					"bw_name" : "Read from L1",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [62.0916748046875, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 9.220093727111816,
					"bw_name" : "Write to L0A",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [123.13834381103516, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 4.610046863555908,
					"bw_name" : "Write to L0B",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [130.83448791503906, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 9.220093727111816,
					"bw_name" : "Write to L1",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [71.74473571777344, 15.380544662475586],
					"ratio" : "0.047412"
				}
			],
			"title" : "Memory Unit(Cube)"
		}, {
			"rooflines" : [{
					"bw" : 4.450546741485596,
					"bw_name" : "FIXP",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [80.51353454589844, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 6.420710563659668,
					"bw_name" : "MTE1",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [66.2754135131836, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 5.207109451293945,
					"bw_name" : "MTE2",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [34.860416412353516, 15.380544662475586],
					"ratio" : "0.084731"
				}, {
					"bw" : 4.450546741485596,
					"bw_name" : "MTE3",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [128.94195556640625, 15.380544662475586],
					"ratio" : "0.047412"
				}
			],
			"title" : "Pipe Line Cube"
		}, {
			"rooflines" : [{
					"bw" : 8.629688262939453,
					"bw_name" : "GM/L1 to L0A",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [123.13834381103516, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 4.073671817779541,
					"bw_name" : "GM/L1 to L0B",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [130.83448791503906, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 4.469531059265137,
					"bw_name" : "L0C to GM",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [80.51353454589844, 15.380544662475586],
					"ratio" : "0.047412"
				}, {
					"bw" : 4.450546741485596,
					"bw_name" : "L1 to GM",
					"computility" : 324.4031982421875,
					"computility_name" : "Cube_FP(100.000000%)",
					"point" : [983.606689453125, 15.380544662475586],
					"ratio" : "0.047412"
				}
			],
			"title" : "Memory Pipe Cube"
		}, {
			"rooflines" : [{
					"bw" : 9.220093727111816,
					"bw_name" : "Read from UB",
					"computility" : 4.444128036499023,
					"computility_name" : "Vec_FP32(75.057518%),Vec_MISC(24.844229%),Vec_S16(0.098255%)",
					"point" : [3.3497071266174316, 0.37334126234054565],
					"ratio" : "0.084008"
				}, {
					"bw" : 36.880374908447266,
					"bw_name" : "UB Read + Write",
					"computility" : 4.444128036499023,
					"computility_name" : "Vec_FP32(75.057518%),Vec_MISC(24.844229%),Vec_S16(0.098255%)",
					"point" : [0.09360750019550323, 0.37334126234054565],
					"ratio" : "0.108143"
				}, {
					"bw" : 36.880374908447266,
					"bw_name" : "Vector Read UB",
					"computility" : 4.444128036499023,
					"computility_name" : "Vec_FP32(75.057518%),Vec_MISC(24.844229%),Vec_S16(0.098255%)",
					"point" : [0.18892647325992584, 0.37334126234054565],
					"ratio" : "0.084008"
				}, {
					"bw" : 36.880374908447266,
					"bw_name" : "Vector Write UB",
					"computility" : 4.444128036499023,
					"computility_name" : "Vec_FP32(75.057518%),Vec_MISC(24.844229%),Vec_S16(0.098255%)",
					"point" : [0.22318574786186218, 0.37334126234054565],
					"ratio" : "0.084008"
				}, {
					"bw" : 9.220093727111816,
					"bw_name" : "Write to UB",
					"computility" : 4.444128036499023,
					"computility_name" : "Vec_FP32(75.057518%),Vec_MISC(24.844229%),Vec_S16(0.098255%)",
					"point" : [1.6373746395111084, 0.37334126234054565],
					"ratio" : "0.084008"
				}
			],
			"title" : "Memory Unit(Vector)"
		}, {
			"rooflines" : [{
					"bw" : 4.142578125,
					"bw_name" : "MTE2 vector",
					"computility" : 4.444128036499023,
					"computility_name" : "Vec_FP32(75.057518%),Vec_MISC(24.844229%),Vec_S16(0.098255%)",
					"point" : [1.6373746395111084, 0.37334126234054565],
					"ratio" : "0.084008"
				}, {
					"bw" : 4.576640605926514,
					"bw_name" : "MTE3 vector",
					"computility" : 4.444128036499023,
					"computility_name" : "Vec_FP32(75.057518%),Vec_MISC(24.844229%),Vec_S16(0.098255%)",
					"point" : [3.3497071266174316, 0.37334126234054565],
					"ratio" : "0.084008"
				}
			],
			"title" : "Pipe Line Vector"
		}
	]
}
)JSON";
#endif // PROFILER_SERVER_DETAILSROOFLINEJSON_H
