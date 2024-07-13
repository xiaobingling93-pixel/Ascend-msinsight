/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import { warning } from 'rc-util/lib/warning';
import * as React from 'react';
import type { GetRowKey, Key } from '../types';

export interface TreeViewModel<T> {
    data: T;
    depth: number;
    subtreeSize: number;
    children: Array<TreeViewModel<T>>;
    parent?: TreeViewModel<T>;
};

/**
 * Inserts a view model for a given data to a given view model, without touching the original data structure.
 * Unused for now.
 *
 * @param node
 * @param child
 */
export function insert<T extends Record<string, unknown>>(
    node: TreeViewModel<T>,
    child: T,
    childrenColumnName: string,
    expandedKeys: Set<Key>,
    getRowKey: GetRowKey<T>
): void {
    const vm = treeMap({record:child, depth:node.depth + 1, childrenColumnName, expandedKeys, getRowKey, parent:node});
    node.children.push(vm);
    let cur: TreeViewModel<T> | undefined = node;
    while (cur) {
        cur.subtreeSize += vm.subtreeSize;
        cur = cur.parent;
    }
}

/**
 * Removes a view model without changing the original data tree structure
 * Unused for now.
 *
 * @param node
 */
export function remove<T>(
    node: TreeViewModel<T>,
): void {
    if (!node.parent) {
        throw new Error('Can\'t remove a root view model');
    }
    const children = node.parent.children;
    children.splice(children.indexOf(node), 1);
    let cur: TreeViewModel<T> | undefined = node.parent;
    while (cur) {
        cur.subtreeSize -= node.subtreeSize;
        cur = cur.parent;
    }
}

interface Itree<T> {
    record: T;
    depth: number;
    childrenColumnName: string;
    expandedKeys: Set<Key>;
    getRowKey: GetRowKey<T>;
    parent?: TreeViewModel<T>;
}

function treeMap<T extends Record<string, unknown>>(treeSet:Itree<T>): TreeViewModel<T> {
    const {record, depth, childrenColumnName, expandedKeys, getRowKey, parent} = treeSet;
    const rootVm = {
        data: record,
        depth,
        subtreeSize: 1,
        children: [] as Array<TreeViewModel<T>>,
        parent,
    };

    const key = getRowKey(record);
    const expanded = expandedKeys?.has(key);

    const childRecords = record[childrenColumnName];
    if (record && Array.isArray(childRecords) && expanded) {
        rootVm.children = childRecords.map(it => treeMap({
            record:it,
            depth:depth + 1,
            childrenColumnName,
            expandedKeys,
            getRowKey,
            parent:rootVm,
        }));
        rootVm.subtreeSize = rootVm.children.reduce((prev, cur) => prev + cur.subtreeSize, 1);
    }

    return rootVm;
}

export function useOrderStatisticTree<T extends Record<string, unknown>>(
    data: T | readonly T[],
    childrenColumnName: string,
    expandedKeys: Set<Key>,
    getRowKey: GetRowKey<T>
): OrderStatisticTree<T> {
    return React.useMemo(() => {
        let tree;
        if (Array.isArray(data)) {
            tree = data.map(it => treeMap({record:it, depth:0, childrenColumnName, expandedKeys, getRowKey}));
        } else {
            tree = treeMap({record:data as T, depth:0, childrenColumnName, expandedKeys, getRowKey});
        }
        return new OrderStatisticTree(tree);
    }, [data, childrenColumnName, expandedKeys, getRowKey]);
}

export class OrderStatisticTree<T> {
    private vm: TreeViewModel<T> | Array<TreeViewModel<T>>;

    constructor(tree: TreeViewModel<T> | Array<TreeViewModel<T>>) {
        this.vm = tree;
    }

    findNodeIndex(selectedNode: T): number {
        let resultIndex = 0;
        const recursiveNodeIndex = (node: Array<TreeViewModel<T>>, originSubTreeIndex: number): number => {
            let subTreeIndex = originSubTreeIndex;
            node.forEach(item => {
                if (item.data === selectedNode) {
                    resultIndex = subTreeIndex;
                    return;
                } else {
                    subTreeIndex --;
                    if (item.children.length !== 0) {
                        subTreeIndex = recursiveNodeIndex(item.children, subTreeIndex);
                    }
                }
            });
            return subTreeIndex;
        };
        let initTreeLength = 0;
        if (!Array.isArray(this.vm)) {
            throw new Error('Unsupported Data');
        }
        this.vm.forEach(item => initTreeLength += item.subtreeSize);
        recursiveNodeIndex(this.vm, initTreeLength);
        return initTreeLength - resultIndex;
    }

    // could be optimized by linking tree nodes in order
    getVisibleData(scrollTop: number, vpHeight: number, rowHeight: number, tolerance: number = 0): Array<TreeViewModel<T>> {
        if (!this.vm || (Array.isArray(this.vm) && this.vm.length === 0)) {
            return [];
        }
        const startIndex = Math.max(0, Math.floor(scrollTop / rowHeight) - tolerance);
        const endIndex = Math.min(this.getTotalCount() - 1, tolerance + Math.floor((scrollTop + vpHeight - 1) / rowHeight));
        const result = [];
        for (let i = startIndex; i <= endIndex; i++) {
            const found = this.findNode(this.vm, i);
            if (!found) {
                warning(false, 'getFlattenData found undefined element');
            } else {
                result.push(found);
            }
        }
        return result;
    }

    getTotalCount(): number {
        if (Array.isArray(this.vm)) {
            return this.vm.reduce((prev, cur) => prev + cur.subtreeSize, 0);
        }
        return this.vm.subtreeSize;
    }

    getTotalHeight(rowHeight: number): number {
        return this.getTotalCount() * rowHeight;
    }

    private findNode(tree: TreeViewModel<T> | Array<TreeViewModel<T>>, index: number): TreeViewModel<T> | undefined {
        if (Array.isArray(tree)) {
            let subTreeIndex = index;
            for (let i = 0; i < tree.length; i++) {
                const subtree = tree[i];
                if (subtree.subtreeSize > subTreeIndex) {
                    return this.findNode(subtree, subTreeIndex);
                }
                subTreeIndex -= subtree.subtreeSize;
            }
            // findNode returning undefined for tree exausted, tree length: ', tree.length, ', index: ', index
            return undefined;
        }
        if (index >= tree.subtreeSize) {
            // findNode returning undefined for index >= tree.subtreeSize
            return undefined;
        }
        if (index === 0) {
            return tree;
        }
        if (!tree.children || tree.children.length === 0) {
            // findNode returning undefined for empty children
            return undefined;
        }
        let subTreeIndex = index - 1; // skipped the root
        let subTree;
        for (let i = 0; i < tree.children.length; i++) {
            subTree = tree.children[i];
            if (subTreeIndex < subTree.subtreeSize) {
                return this.findNode(subTree, subTreeIndex);
            }
            subTreeIndex -= subTree.subtreeSize;
        }
        // findNode returning undefined
        return undefined;
    }
}
