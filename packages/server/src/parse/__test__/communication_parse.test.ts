/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { parseCommunicationFile, saveClusterBaseInfo } from '../communication_parser';
import path from 'path';
import { COMMUNICATION_BAND_WIDTH_TABLE, COMMUNICATION_TIME_INFO_TABLE } from '../../common/sql_constant';
import { CLUSTER_DATABASE } from '../../database/tableManager';

describe('communication parse test', () => {
    it('test parse file ', async function () {
        const testFile = path.join(__dirname, 'test_communication_file.json');
        const fileArr = [testFile];
        await parseCommunicationFile(fileArr);
        const num1 = CLUSTER_DATABASE.getTotal(COMMUNICATION_TIME_INFO_TABLE);
        const num2 = CLUSTER_DATABASE.getTotal(COMMUNICATION_BAND_WIDTH_TABLE);
        expect(num1).toBe(undefined);
        expect(num2).toBe(undefined);
    });

    it('test saveClusterBaseInfo', function () {
        saveClusterBaseInfo(__dirname);
    });
});
