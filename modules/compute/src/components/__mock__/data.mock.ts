/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { ICoreOccupancy } from '../detail/CoreOccupancy/Index';

const mockCoreData: ICoreOccupancy = {
    soc: 'Ascend910B4',
    opType: 'mix',
    advice: 'Core0 XXXX \nxxxxx',
    opDetails: [],
};

for (let i = 0; i < 24; i++) {
    mockCoreData.opDetails.push({
        coreId: i, // core序号
        subCoreDetails: [
            {
                subCoreName: 'Cube0', // sub core 名字+序号：cubeX，vectoreX
                cycles: {
                    value: parseInt((612 / (i + 1)).toFixed(0)), //
                    level: (i % 10) + 1, // 颜色的级别，0~10,0代表没有数据或者数据为0，显示为灰色；1~10代表十种颜色区间
                },
                throughput: {
                    value: 100, // 核吞吐数据 （GB/s）
                    level: 5,
                },
                cacheHitRate: {
                    value: 5, //  L2cache命中率 (%)
                    level: 1,
                },
            },
            {
                subCoreName: '<div>c</div>',
                cycles: {
                    value: 2,
                    level: 2,
                },
                throughput: {
                    value: 100,
                    level: 5,
                },
                cacheHitRate: {
                    value: 5,
                    level: 1,
                },
            },
            {
                subCoreName: 'Vector0',
                cycles: {
                    value: parseInt((1000 / (i + 1)).toFixed(0)),
                    level: (i % 3) + 5,
                },
                throughput: {
                    value: 100,
                    level: 5,
                },
                cacheHitRate: {
                    value: 5,
                    level: 1,
                },
            },
        ],
    });
}

export const coreData = mockCoreData;
