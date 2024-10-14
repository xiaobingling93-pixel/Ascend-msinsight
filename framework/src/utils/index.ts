/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

export const getOperatingSystem = function ():string {
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

const removePrototypePollution = (obj: any): void => {
    if (obj && typeof obj === 'object') {
        for (let key in obj) {
            if (key === '__proto__' || key === 'constructor') {
                delete obj[key];
            } else if (typeof obj[key] === 'object') {
                removePrototypePollution(obj[key]);
            }
        }
    }
};

export const safeJSONParse = (str: any, defaultValue: any = null): any => {
    try {
        const res = JSON.parse(str);
        removePrototypePollution(res);
        return res;
    } catch (error) {
        return defaultValue;
    }
};