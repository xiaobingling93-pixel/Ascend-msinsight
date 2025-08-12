/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import type { IframeHTMLAttributes } from 'react';

export interface ModuleConfig {
    name: string;
    requestName: Lowercase<string>;
    attributes: IframeHTMLAttributes<HTMLIFrameElement>;
    isDefault?: boolean;
    isCluster?: boolean;
    isCompute?: boolean;
    isJupyter?: boolean;
    isLeaks?: boolean;
    isIE?: boolean;
    isRL?: boolean;
    hasCachelineRecords?: boolean;
    isOnlyTraceJson?: boolean;
};

const isDev = process.env.REACT_APP_ENV === 'development';
export const modulesConfig: ModuleConfig[] = [
    {
        name: 'Timeline',
        requestName: 'timeline',
        attributes: {
            src: isDev ? 'http://localhost:3000/' : './plugins/Timeline/index.html',
        },
        isDefault: true,
        isCluster: true,
        isCompute: true,
        isJupyter: true,
        isOnlyTraceJson: true,
        isIE: true,
        isLeaks: true,
    },
    {
        name: 'Memory',
        requestName: 'memory',
        attributes: {
            src: isDev ? 'http://localhost:3001/' : './plugins/Memory/index.html',
        },
        isDefault: true,
        isCluster: true,
        isJupyter: true,
    },
    {
        name: 'Operator',
        requestName: 'operator',
        attributes: {
            src: isDev ? 'http://localhost:3002/' : './plugins/Operator/index.html',
        },
        isDefault: true,
        isCluster: true,
        isJupyter: true,
    },
    {
        name: 'Summary',
        requestName: 'summary',
        attributes: {
            src: isDev ? 'http://localhost:3003/summary.html' : './plugins/Cluster/summary.html',
        },
        isCluster: true,
    },
    {
        name: 'Communication',
        requestName: 'communication',
        attributes: {
            src: isDev ? 'http://localhost:3003/communication.html' : './plugins/Cluster/communication.html',
        },
        isCluster: true,
    },
    {
        name: 'Source',
        requestName: 'compute',
        attributes: {
            src: isDev ? 'http://localhost:3004/source.html' : './plugins/Compute/source.html',
        },
        isCompute: true,
    },
    {
        name: 'Details',
        requestName: 'compute',
        attributes: {
            src: isDev ? 'http://localhost:3004/detail.html' : './plugins/Compute/detail.html',
        },
        isCompute: true,
    },
    {
        name: 'Cache',
        requestName: 'compute',
        attributes: {
            src: isDev ? 'http://localhost:3004/cache.html' : './plugins/Compute/cache.html',
        },
        hasCachelineRecords: true,
    },
    {
        name: 'Jupyter',
        requestName: 'jupyter',
        attributes: {
            src: isDev ? 'http://localhost:3005/' : './plugins/Jupyter/index.html',
        },
        isJupyter: true,
    },
    {

        name: 'Statistic',
        requestName: 'statistic',
        attributes: {
            src: isDev ? 'http://localhost:3006/' : './plugins/Statistic/index.html',
        },
        isIE: true,
    },
    {
        name: 'Leaks',
        requestName: 'leaks',
        attributes: {
            src: isDev ? 'http://localhost:3007/' : './plugins/Leaks/index.html',
        },
        isLeaks: true,
    },
    {
        name: 'RL',
        requestName: 'rl',
        attributes: {
            src: isDev ? 'http://localhost:3008/' : './plugins/RL/index.html',
        },
        isRL: true,
    },
];
