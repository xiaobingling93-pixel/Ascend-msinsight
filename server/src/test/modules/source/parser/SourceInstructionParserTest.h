/*
* Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

#ifndef PROFILER_SERVER_SOURCEINSTRUCTIONPARSERTEST_H
#define PROFILER_SERVER_SOURCEINSTRUCTIONPARSERTEST_H

#include <string>

const std::string_view SOURCE_INSTRUCTIONS_JSON = R"(
{
	"Cores" : [
		"core0.cubecore0", "core0.veccore0"
	],
	"Instructions Dtype" : {
		"Instructions" : {
			"string" : 0,
			"int" : 1,
            "float" : 2,
            "string list" : 0,
			"int list" : 1,
            "float list" : 2
		}
	},
	"Instructions" : [{
			"string" : "0x1269f000",
			"int" : 1,
			"float" : 1.2,
			"string list" : [
				"aa", "bb"
			],
			"int list" : [
				1, 2
			],
			"float list" : [
				1.2, 2.2
			]
		}, {
			"string" : "0x1269f001",
			"int" : 11,
			"float" : 1.2,
			"string list" : [
				"aa1", "bb1"
			],
			"int list" : [
				11, 22
			],
			"float list" : [
				11.2, 12.2
			]
		}
	]
}
)";

#endif // PROFILER_SERVER_SOURCEINSTRUCTIONPARSERTEST_H
