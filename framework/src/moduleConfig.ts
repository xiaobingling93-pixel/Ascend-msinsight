import type { IframeHTMLAttributes } from "vue";

export interface ModuleConfig {
    name: string;
    requestName: Lowercase<string>;
    attributes: IframeHTMLAttributes;
    isDefault?: boolean;
};

export const modulesConfig: ModuleConfig[] = [
    {
        name: 'Timeline View',
        requestName: 'timeline',
        attributes: {
            src: './plugins/Timeline/index.html'
        },
        isDefault: true,
    },
    {
        name: 'summary',
        requestName: 'summary',
        attributes: {
            src: './plugins/Cluster/summary.html'
        }
    },
    {
        name: 'communication',
        requestName: 'communication',
        attributes: {
            src: './plugins/Cluster/communication.html'
        }
    },
    {
        name: 'Memory',
        requestName: 'memory',
        attributes: {
            src: './plugins/Memory/index.html'
        },
        isDefault: true
    }
];
