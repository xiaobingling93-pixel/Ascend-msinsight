<script setup lang="ts">
import { toRaw } from 'vue';
import type Node from 'element-plus/es/components/tree/src/model/node';
import type { TreeNodeType } from '@/components/MenuTree/types';
import { useDataSources } from '@/stores/dataSource';
import Delete from './DeleteIcon.vue';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import { ElMessageBox } from 'element-plus';

const props = defineProps<{ data: TreeNodeType; isDeleteAll: boolean; node: Node }>();
const store = useDataSources();
const [DeleteAll, Cancel, Confirm, DeleteItemConfirmDescribe, DeleteAllConfirmDescribe] = useWatchTranslation([
    'Delete All',
    'Cancel',
    'Confirm',
    'DeleteItemConfirmDescribe',
    'DeleteAllConfirmDescribe',
]);

const handleDeleteSingle = () => {
    const parentData = props.node.parent.data;
    const parentIndex = store.menuTree.findIndex((data) => data === toRaw(parentData));
    const dataIndex = store.menuTree[parentIndex]?.children?.findIndex((data) => data === toRaw(props.data));
    if (dataIndex !== undefined) {
        store.removeSingle(parentIndex, dataIndex);
    }
};
const handleDelete = () => {
    const current = store.menuTree.findIndex((data) => data === toRaw(props.data));
    store.remove(current);
};

const handleDeleteAllClick = () => {
    ElMessageBox.confirm(DeleteAllConfirmDescribe.value, {
        title: DeleteAll.value,
        confirmButtonText: Confirm.value,
        cancelButtonText: Cancel.value,
    }).then(() => {
        handleDelete();
    });
};
</script>

<template>
    <Delete v-if="isDeleteAll" @click.stop="handleDeleteAllClick" />
    <el-popconfirm width="200" v-else :hide-icon="true" @confirm="handleDeleteSingle" :hide-after="0" :title="DeleteItemConfirmDescribe">
        <template #reference>
            <Delete @click.stop />
        </template>
    </el-popconfirm>
</template>
