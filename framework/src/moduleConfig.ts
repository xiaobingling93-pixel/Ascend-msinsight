import type { IframeHTMLAttributes } from "vue";

export interface ModuleConfig {
    name: string;
    requestName: Lowercase<string>;
    attributes: IframeHTMLAttributes;
    isDefault?: boolean;
};

export const modulesConfig: ModuleConfig[] = [
    {
        name: 'Timeline',
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
            src: 'http://localhost:3000/summary.html'
        }
    },
    {
        name: 'communication',
        requestName: 'communication',
        attributes: {
            src: 'http://localhost:3000/communication.html'
        }
    },
];

