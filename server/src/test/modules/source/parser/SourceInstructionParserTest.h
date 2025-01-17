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
			"skip" : 0,
			"int" : 1,
            "float" : 2,
            "string" : 3,
			"int list" : 1,
            "float list" : 2,
            "string list" : 3
		}
	},
	"Instructions" : [{
            "skip" : "skip",
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
            "skip" : "skip",
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

const std::string_view SOURCE_API_FILE_JSON = R"(
{
	"Cores" : [
		"core0.cubecore0", "core0.veccore0"
	],
	"Files Dtype" : {
		"Lines" : {
			"Address Range" : 0,
			"int" : 1,
            "float" : 2,
			"string" : 3,
            "string list" : 3,
			"int list" : 1,
            "float list" : 2
		}
	},
    "Files":[
        {
            "Source": "a.cpp",
            "Lines" :
            [
                {
                    "Address Range":[
                        ["a", "b"], ["c", "d"]
                    ],
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
                },
                {
                    "Address Range":[
                        ["e", "f"], ["g", "h"]
                    ],
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
        },
        {
            "Source": "b.cpp",
            "Lines" :
            [
                {
                    "Address Range":[
                        ["aa", "bb"], ["cc", "dd"]
                    ],
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
                },
                {
                    "Address Range":[
                        ["ae", "af"], ["ag", "ah"]
                    ],
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
    ]
}
)";

#endif // PROFILER_SERVER_SOURCEINSTRUCTIONPARSERTEST_H
