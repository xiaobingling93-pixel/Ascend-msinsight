<script setup lang="ts">
import { defineProps } from 'vue';
import TreeNode from './TreeNode.vue';
import type { TreeNodeType } from './types';
const props = defineProps<{
    dataSource: TreeNodeType[];
}>();
const getOriginedDataSource = (dataSource?: TreeNodeType[]): TreeNodeType[] | undefined => {
    if (dataSource === undefined || dataSource.length === 0) {
        return undefined;
    }
    return dataSource.map(({ children, ...data }) => ({
        ...data,
        children: getOriginedDataSource(children),
        origin: props.dataSource,
    }));
};
const originedDataSource = getOriginedDataSource(props.dataSource);
</script>
<template>
    <div class="menu-tree">
        <TreeNode
            v-for="(item, index) in originedDataSource"
            :key="index"
            :data="item"
            :tier="0"
        />
    </div>
</template>
<style scoped>
.menu-tree {
    padding: 0.5rem 0.8rem;
}
</style>
