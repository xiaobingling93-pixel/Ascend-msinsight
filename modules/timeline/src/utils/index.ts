/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { type InsightUnit } from '../entity/insight';
import { forEach } from 'lodash';

export const getOperatingSystem = function (): string {
    const userAgent = navigator.userAgent.toLowerCase();

    if (userAgent.includes('windows')) {
        return 'Windows';
    } else if (userAgent.includes('macintosh') || userAgent.includes('mac os')) {
        return 'Mac OS';
    } else if (userAgent.includes('linux')) {
        return 'Linux';
    } else {
        return 'Unknown';
    }
};

export const getRootUnit = (units: InsightUnit[]): InsightUnit[] => {
    const result: InsightUnit[] = [];
    forEach(units, (unit) => {
        if (unit.parent) {
            if (!result.includes(unit.parent)) {
                result.push(unit.parent);
            }
        } else {
            result.push(unit);
        }
    });
    return result;
};
