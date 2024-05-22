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
        name: 'Timeline View',
        requestName: 'timeline',
        attributes: {
            src: './plugins/Timeline/index.html'
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
            src: './plugins/Memory/index.html'
        },
        isDefault: true,
        isCluster: true,
        isJupyter: true,
    },
    {
        name: 'Operator',
        requestName: 'operator',
        attributes: {
            src: './plugins/Operator/index.html'
        },
        isDefault: true,
        isCluster: true,
        isJupyter: true,
    },
    {
        name: 'Summary',
        requestName: 'summary',
        attributes: {
            src: './plugins/Cluster/summary.html'
        },
        isCluster: true,
    },
    {
        name: 'Communication',
        requestName: 'communication',
        attributes: {
            src: './plugins/Cluster/communication.html'
        },
        isCluster: true,
    },
    {
        name: 'Source',
        requestName: 'compute',
        attributes: {
            src: './plugins/Compute/source.html',
        },
        isCompute: true,
    },
    {
        name: 'Details',
        requestName: 'compute',
        attributes: {
            src: './plugins/Compute/detail.html',
        },
        isCompute: true,
    },
    {
        name: 'Jupyter',
        requestName: 'jupyter',
        attributes: {
            src: './plugins/Jupyter/index.html',
        },
        isJupyter: true,
    },
];
