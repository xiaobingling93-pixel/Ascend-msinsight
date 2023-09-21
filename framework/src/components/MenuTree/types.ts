export type TreeNodeType = {
    label: string;
    cancelable?: boolean;
    children?: TreeNodeType[];
    origin?: TreeNodeType[];
};

export const indent = 1.2;
