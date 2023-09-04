<script setup lang="ts">
import { defineProps, ref } from 'vue';
import type { TreeNodeType } from './types';
import Expand from './ExpandIcon.vue';
import Delete from './DeleteIcon.vue';
import { indent } from './types';

const props = defineProps<{ data: TreeNodeType; tier: number }>();
let isExpanded = ref(true);
const hasExpandIcon = props.data.children && props.data.children.length !== 0;
const occupy = hasExpandIcon ? 0 : 1;
const nextTier = props.tier + 1;
const handleClick = () => {
    isExpanded.value = !isExpanded.value;
};
const handleDelete = () => {
    console.log('delete');
};
</script>
<template>
    <div class="tree-node-line" :style="`padding-left: ${indent * (tier + occupy)}rem`">
        <Expand @click="handleClick" :isExpanded="isExpanded" v-if="hasExpandIcon" />
        <div class="menu-tree-node">
            {{ data.content }}
        </div>
        <Delete @click="handleDelete" v-if="data.cancelable" />
    </div>
    <template v-if="isExpanded">
        <TreeNode
            v-for="(item, index) in data.children"
            :key="`${item}-${index}`"
            :data="item"
            :tier="nextTier"
        >
        </TreeNode>
    </template>
</template>
<style scoped>
.tree-node-line {
    display: flex;
    justify-content: space-between;
    align-items: center;
    max-width: 200px;
    border-radius: var(--border-radius);
}

.tree-node-line:hover {
    background-color: var(--color-background-medium);
}

@media (min-width: 1024px) {
    .tree-node-line {
        max-width: unset;
    }
}

.tree-node-line .menu-tree-node {
    flex-grow: 1;
}
</style>
