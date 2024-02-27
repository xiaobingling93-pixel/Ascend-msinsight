<script setup lang="ts">
import {ref, toRaw} from 'vue';
import type Node from 'element-plus/es/components/tree/src/model/node';
import type { TreeNodeType } from '@/components/MenuTree/types';
import { useDataSources } from '@/stores/dataSource';
import Delete from './DeleteIcon.vue';

const props = defineProps<{ data: TreeNodeType; isDeleteAll: boolean; node: Node }>();
const dialogVisible = ref(false);
const store = useDataSources();
const handleDeleteSingle = () => {
    const parentData = props.node.parent.data;
    const parentIndex = store.menuTree.findIndex(data => data === toRaw(parentData));
    const dataIndex = store.menuTree[parentIndex].children?.findIndex(data => data === toRaw(props.data));
    if (dataIndex !== undefined) {
        store.removeSingle(parentIndex, dataIndex);
    }
};
const handleDelete = () => {
  const current = store.menuTree.findIndex(data => data === toRaw(props.data));
  store.remove(current);
};
const handleDeleteAll = () => {
  handleDelete();
  dialogVisible.value = false;
};
</script>

<template>
    <el-popconfirm width="200" v-if="!isDeleteAll" :hide-icon="true" @confirm="handleDeleteSingle" :hide-after="0"
        title="Are you sure to delete this?">
        <template #reference>
            <Delete />
        </template>
    </el-popconfirm>
    <Delete v-if="isDeleteAll" @click="dialogVisible = true" />
    <el-dialog v-model="dialogVisible" title="Delete All" width="20%" :show-close="false" :align-center="true">
        <span>Are you sure to delete All</span>
        <template #footer>
            <span class="dialog-footer">
                <el-button @click="dialogVisible = false">Cancel</el-button>
                <el-button type="primary" @click="handleDeleteAll"> Confirm </el-button>
            </span>
        </template>
    </el-dialog>
</template>
<style>
.el-dialog__footer button{
    padding:0 8px;
}
</style>