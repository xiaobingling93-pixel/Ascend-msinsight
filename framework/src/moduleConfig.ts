import type {IframeHTMLAttributes} from "vue";

export interface ModuleConfig {
    name: string;
    requestName: Lowercase<string>;
    attributes: IframeHTMLAttributes;
    isDefault?: boolean;
    isCluster?: boolean;
    isCompute?: boolean;
    isJupyter?: boolean;
};

export const modulesConfig: ModuleConfig[] = [
    {
        name: 'Timeline',
        requestName: 'timeline',
        attributes: {
            src: import.meta.env.DEV ? 'http://localhost:3000/' : './plugins/Timeline/index.html',
        },
        isDefault: true,
        isCluster: true,
        isCompute: true,
        isJupyter: true,
    },
    {
        name: 'Memory',
        requestName: 'memory',
        attributes: {
            src: import.meta.env.DEV ? 'http://localhost:3001/' : './plugins/Memory/index.html',
        },
        isDefault: true,
        isCluster: true,
        isJupyter: true,
    },
    {
        name: 'Operator',
        requestName: 'operator',
        attributes: {
            src: import.meta.env.DEV ? 'http://localhost:3002/' : './plugins/Operator/index.html',
        },
        isDefault: true,
        isCluster: true,
        isJupyter: true,
    },
    {
        name: 'Summary',
        requestName: 'summary',
        attributes: {
            src: import.meta.env.DEV ? 'http://localhost:3003/summary.html' : './plugins/Cluster/summary.html',
        },
        isCluster: true,
    },
    {
        name: 'Communication',
        requestName: 'communication',
        attributes: {
            src: import.meta.env.DEV ? 'http://localhost:3003/communication.html' : './plugins/Cluster/communication.html',
        },
        isCluster: true,
    },
    {
        name: 'Source',
        requestName: 'compute',
        attributes: {
            src: import.meta.env.DEV ? 'http://localhost:3004/source.html' : './plugins/Compute/source.html',
        },
        isCompute: true,
    },
    {
        name: 'Details',
        requestName: 'compute',
        attributes: {
            src: import.meta.env.DEV ? 'http://localhost:3004/detail.html' : './plugins/Compute/detail.html',
        },
        isCompute: true,
    },
    {
        name: 'Jupyter',
        requestName: 'jupyter',
        attributes: {
            src: import.meta.env.DEV ? 'http://localhost:3005/' : './plugins/Jupyter/index.html',
        },
        isJupyter: true,
    },
];
