/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { CLUSTER_DATABASE } from '../tableManager';

describe('test cluster database', () => {
    it('test query base info', async () => {
        await CLUSTER_DATABASE.createClusterTable();
        CLUSTER_DATABASE.insertClusterBaseInfo([ 1, 'filePath', '[1,2,3]', '[11,22]', 42131, 123213 ]);
        const baseInfo = await CLUSTER_DATABASE.queryBaseInfo();
        expect(baseInfo).toEqual([
            {
                filePath: '1',
                ranks: 'filePath',
                steps: '[1,2,3]',
                dataSize: 123213,
            },
        ]);
    });
});
