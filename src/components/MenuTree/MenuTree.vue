<script setup lang="ts">
import { defineProps } from 'vue';
import type { TreeNodeType } from './types';
import LocalIcon from '@/components/icons/loaclImport_icon.vue';
import DeletePopConfirm from '@/components/MenuTree/DeletePopConfirm.vue';

const props = defineProps<{
    dataSource: TreeNodeType[];
}>();

</script>
<template>
    <div class="menu-tree">
        <el-tree :data="props.dataSource" node-key="id" :default-expand-all="true">
            <template #default="{ node, data }">
                <div class="contentNode">
                    <span class="contentBody">
                        <LocalIcon v-if="node.level === 1" />
                        <span v-if="node.level === 1" class="contentText">
                            {{ node.label }}
                        </span>
                        <span v-if="node.level === 2" class="contentNodeText">
                            {{ node.label }}
                        </span>
                    </span>
                  <DeletePopConfirm v-if="node.level === 1" :data="data" :is-delete-all="node.level === 1"/>
                </div>
            </template>
        </el-tree>
    </div>
</template>
<style scoped>
.menu-tree {
    padding: 0.5rem 0.8rem;
}
.el-tree {
  --el-tree-node-hover-bg-color: var(--color-border-hover);
  color: var(--dataPath-color) !important;
}

::v-deep(.contentNode) {
    position: relative;
    width: 100%;
    display: flex;
    user-select: none;
    justify-content: space-between;
}
::v-deep(.contentBody) {
    display: flex;
    align-items: center;
}
::v-deep(.treeDeleteIcon) {
    position: absolute;
    right: 0;
    transition: all 0.5s ease-out;
}
::v-deep(.contentText) {
    padding: 4px 0 0 2px;
    font-size: 14px;
    color: var(--treeContent-color);
    line-height: 24px;
    font-weight: 700;
}
::v-deep(.contentNodeText) {
    padding: 0 0 0 2px;
    font-size: 12px;
    color: var(--treeNode-color);
    font-weight: 400;
}
</style>
