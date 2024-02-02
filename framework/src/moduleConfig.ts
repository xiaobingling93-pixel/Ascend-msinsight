import type {IframeHTMLAttributes} from "vue";

export interface ModuleConfig {
    name: string;
    requestName: Lowercase<string>;
    attributes: IframeHTMLAttributes;
    isDefault?: boolean;
    isCluster?: boolean;
    isCompute?: boolean;
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
    },
    {
        name: 'Memory',
        requestName: 'memory',
        attributes: {
            src: './plugins/Memory/index.html'
        },
        isDefault: true,
        isCluster: true,
    },
    {
        name: 'Operator',
        requestName: 'operator',
        attributes: {
            src: './plugins/Operator/index.html'
        },
        isDefault: true,
        isCluster: true,
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
        name: 'Compute',
        requestName: 'compute',
        attributes: {
            src: './plugins/Compute/index.html',
        },
        isCompute: true,
    },
];
