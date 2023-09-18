import type { IframeHTMLAttributes } from "vue";

export interface ModuleConfig {
    name: string;
    requestName: Lowercase<string>;
    attributes: IframeHTMLAttributes;
};

export const modulesConfig: ModuleConfig[] = [
    {
        name: 'Timeline',
        requestName: 'timeline',
        attributes: {
            src: './plugins/Timeline/index.html'
        },
    },
    {
        name: 'test',
        requestName: 'test',
        attributes: {
            src: './plugins/test/index.html'
        }
    }
];

