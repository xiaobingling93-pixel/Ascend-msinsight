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

export const safeJSONParse = (str: any, defaultValue: any = null): any => {
    try {
        return JSON.parse(str);
    } catch (error) {
        return defaultValue;
    }
};