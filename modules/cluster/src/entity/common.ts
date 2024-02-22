
export type TimeStamp = number;

export type TreeNode<T> = T & {
    children?: Array<TreeNode<T>>;
};

export const level = Symbol('unitLevel');
export type PreOrderFlattenOptions<T> = {
    // nodes that should always be flattened, but not appearing in the result, and not counted as one level
    bypass?: (node: TreeNode<T>) => boolean;

    // nodes that are to be flattened normally
    when?: (node: TreeNode<T>) => boolean;

    // nodes that are to be flattened but should not appear in the result
    exclude?: (node: TreeNode<T>) => boolean;
};
export function preOrderFlatten<T>(tree: Array<TreeNode<T>>, currentLevel: number, options?: PreOrderFlattenOptions<T>): T[] {
    tree.forEach(node => {
        if ((node as any)[level] === undefined) {
            (node as any)[level] = currentLevel;
        }
    });
    return tree.flatMap(node => {
        if (options?.bypass?.(node) ?? false) {
            return node.children ? [...preOrderFlatten(node.children, currentLevel, options)] : [];
        } else {
            const self = (options?.exclude?.(node) ?? false) ? [] : [node];
            if (node.children && (options?.when?.(node) ?? true)) {
                return [...self, ...preOrderFlatten(node.children, currentLevel + 1, options)];
            } else {
                return self;
            }
        }
    });
}

export type ElementType<T> = T extends Array<infer U> ? U : never;

export type AtomicObjectElementType<T> = T extends Array<infer U> ? U extends number | string | boolean ? T : AtomicObjectElementType<U> : T;

export type KeysMatching<T extends object, U> = {
    [K in keyof T]-?: T[K] extends U ? K : never
}[keyof T];
