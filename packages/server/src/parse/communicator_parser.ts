/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { Client } from '../types';
import path from 'path';
import fs from 'fs';
import _ from 'lodash';
import { getLoggerByName } from '../logger/loggger_configure';

type communicator = {
    name: string;
    ranks: number[];
    value: string;
};

const logger = getLoggerByName('parseCommunicationGroupHandler', 'info');

export async function parseCommunicationGroupHandler(req: { path: string }, client?: Client): Promise<Record<string, unknown>> {
    const groupJson = path.join(req.path, 'cluster_analysis_output', 'communication_group.json');
    const result: {ppGroups: communicator[]; tpOrDpGroups: communicator[]; defaultPPSize: number} = { ppGroups: [], tpOrDpGroups: [], defaultPPSize: 0 };
    if (fs.existsSync(groupJson)) {
        const { collective, p2p }: {collective: number[][]; p2p: number[][]} = JSON.parse(fs.readFileSync(groupJson, 'utf8'));
        if (collective === undefined || p2p === undefined) {
            logger.error('The file data is incorrect.');
            return result;
        }
        const sortFunc = (a: number[], b: number[]): number => a.length !== b.length ? a.length - b.length : a[0] - b[0];
        result.defaultPPSize = p2p.length;
        p2p.sort(sortFunc).forEach((value, key) => {
            _.pull(collective, _.find(collective, data => numberArrayEqual(data, value)));
            return result.ppGroups.push({ name: 'stage' + key.toString(), ranks: value, value: `(${value.join(', ')}` + (value.length > 1 ? ')' : ',)') });
        });
        collective.sort(sortFunc).forEach((value, key) => {
            return result.tpOrDpGroups.push({ name: 'tpOrDp' + key.toString(), ranks: value, value: `(${value.join(', ')}` + (value.length > 1 ? ')' : ',)') });
        });
    } else {
        logger.error('The file does not exist.');
    }
    return result;
}

function numberArrayEqual(array1: number[], array2: number[]): boolean {
    if (array1.length === array2.length) {
        for (let i = 0; i < array1.length; i++) {
            if (array1[i] !== array2[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}
