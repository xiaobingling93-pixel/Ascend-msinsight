/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { parseCommunicationFile, saveClusterBaseInfo } from '../communication_parser';
import path from 'path';
import { COMMUNICATION_BAND_WIDTH_TABLE, COMMUNICATION_TIME_INFO_TABLE } from '../../common/sql_constant';
import { CLUSTER_DATABASE } from '../../database/tableManager';

describe('communication parse test', () => {
    test('test parse file ', async function () {
        const testFile = path.join(__dirname, 'test_communication_file.json');
        const fileArr = [testFile];
        await CLUSTER_DATABASE.createClusterTable();
        await parseCommunicationFile(fileArr);
        setTimeout(async () => {
            const num1 = await CLUSTER_DATABASE.getTotal(COMMUNICATION_TIME_INFO_TABLE);
            const num2 = await CLUSTER_DATABASE.getTotal(COMMUNICATION_BAND_WIDTH_TABLE);
            await expect(num1).toEqual({ num: 2 });
            await expect(num2).toEqual({ num: 8 });
            CLUSTER_DATABASE.close();
        }, 1000);
    });

    it('test saveClusterBaseInfo', async function () {
        saveClusterBaseInfo(__dirname);
    });
});
