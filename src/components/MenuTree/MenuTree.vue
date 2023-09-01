<script setup lang="ts">
import { defineProps } from 'vue';
import TreeNode from './TreeNode.vue';
import type { TreeNodeType } from './types';
const props = defineProps<{
    dataSource: TreeNodeType[];
}>();
</script>
<template>
    <div class="menu-tree">
        <TreeNode v-for="(item, index) in props.dataSource" :key="index" :data="item" :tier="0" />
    </div>
</template>
<style scoped>
.menu-tree {
    padding: 0.5rem 0.8rem;
}
</style>
