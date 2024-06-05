export type TreeNodeType = {
    id: number;
    projectName: string;
    label: string;
    cancelable?: boolean;
    children?: TreeNodeType[];
    origin?: TreeNodeType[];
};

export const indent = 1.2;
