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
import { ICoreOccupancy } from '../detail/CoreOccupancy/Index';

const mockCoreData: ICoreOccupancy = {
    soc: 'Ascend910B4',
    opType: 'mix',
    advice: 'Core0 XXXX \nxxxxx',
    opDetails: [],
};

for (let i = 0; i < 16; i++) {
    mockCoreData.opDetails.push({
        coreId: i, // core序号
        subCoreDetails: [
            {
                subCoreName: 'Cube0', // sub core 名字+序号：cubeX，vectoreX
                cycles: {
                    value: {
                        compare: parseInt((612 / (i + 1)).toFixed(0)),
                        baseline: 0,
                        diff: 0,
                    }, //
                    level: (i % 10) + 1, // 颜色的级别，0~10,0代表没有数据或者数据为0，显示为灰色；1~10代表十种颜色区间
                },
                throughput: {
                    value: {
                        compare: 100,
                        baseline: 0,
                        diff: 0,
                    }, // 核吞吐数据 （GB/s）
                    level: 5,
                },
                cacheHitRate: {
                    value: {
                        compare: 5,
                        baseline: 0,
                        diff: 0,
                    }, //  L2cache命中率 (%)
                    level: 1,
                },
            },
            {
                subCoreName: '<div>c</div>',
                cycles: {
                    value: {
                        compare: 2,
                        baseline: 0,
                        diff: 0,
                    },
                    level: 2,
                },
                throughput: {
                    value: {
                        compare: 100,
                        baseline: 0,
                        diff: 0,
                    },
                    level: 5,
                },
                cacheHitRate: {
                    value: {
                        compare: 5,
                        baseline: 0,
                        diff: 0,
                    },
                    level: 1,
                },
            },
            {
                subCoreName: 'Vector0',
                cycles: {
                    value: {
                        compare: parseInt((1000 / (i + 1)).toFixed(0)),
                        baseline: 0,
                        diff: 0,
                    },
                    level: (i % 3) + 5,
                },
                throughput: {
                    value: {
                        compare: 100,
                        baseline: 0,
                        diff: 0,
                    },
                    level: 5,
                },
                cacheHitRate: {
                    value: {
                        compare: 5,
                        baseline: 0,
                        diff: 0,
                    },
                    level: 1,
                },
            },
        ],
    });
}

export const coreData = mockCoreData;
