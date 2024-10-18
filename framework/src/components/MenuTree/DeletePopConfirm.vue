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
const [DeleteItemConfirmDescribe, DeleteProjectConfirmDescribe] = useWatchTranslation(['DeleteItemConfirmDescribe', 'DeleteProjectConfirmDescribe']);

const handleSingleDelete = () => {
    const parentData = props.node.parent.data;
    const parentIndex = store.menuTree.findIndex((data) => data === toRaw(parentData));
    const dataIndex = store.menuTree[parentIndex]?.children?.findIndex((data) => data === toRaw(props.data));
    if (dataIndex !== undefined) {
        store.removeSingle(parentIndex, dataIndex);
    }
};
const handleAllDelete = () => {
    const current = store.menuTree.findIndex((data) => data === toRaw(props.data));
    store.remove(current);
};

const handleDelete = () => {
    if (props.isDeleteAll) {
        handleAllDelete();
    } else {
        handleSingleDelete();
    }
};
</script>

<template>
    <el-popconfirm
        width="200"
        :hide-icon="true"
        @confirm="handleDelete"
        :hide-after="0"
        :title="isDeleteAll ? DeleteProjectConfirmDescribe : DeleteItemConfirmDescribe"
    >
        <template #reference>
            <Delete @click.stop />
        </template>
    </el-popconfirm>
</template>
