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
        name: 'test',
        requestName: 'test',
        attributes: {
            src: './plugins/test/index.html'
        }
    }
];

