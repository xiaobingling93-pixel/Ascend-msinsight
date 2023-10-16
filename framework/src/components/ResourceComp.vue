<script setup lang="ts">
import { ref, reactive, watch, onMounted, nextTick } from 'vue';
import type Node from 'element-plus/es/components/tree/src/model/node';
import FolderIcon from '@/components/icons/folder_icon.vue';
import FileIcon from '@/components/icons/file_icon.vue';
import { useResource, type ResourceItem } from '@/stores/resourceComp';
import { LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import { useDataSources } from '@/stores/dataSource';

const { confirm } = useDataSources();

const treeRef = ref()

const { resourceState, loadFiles, setCurrentPath } = useResource();

const defaultProps = {
    label: 'name',
    children: 'children',
    isLeaf: 'leaf',
}
let isUpdateLoading = false;
let defalultExpandedKeysSet: Set<string> = new Set();

const state = reactive({
    defalultExpandedKeys: [] as string[],
    inputPath: "",
})

const updateData = async (path: string,node: Node) => {
    if (isUpdateLoading) {
        return;
    }
    isUpdateLoading = true;
    const oldData = resourceState.resourceTotal[path];
    if (oldData) {
        const newData = await loadFiles(path);
        checkData(newData, oldData, node)
    }
    isUpdateLoading = false;
}

const checkData = (newData: ResourceItem[], oldData: ResourceItem[], node: Node) => {
    const newPaths = newData.map(item => item.path);
    const oldPaths = oldData.map(item => item.path);
    const addData = newData.filter(item => !oldPaths.includes(item.path));
    const deleteDtat = oldData.filter(item => !newPaths.includes(item.path));
    deleteDtat.forEach(item => {
        treeRef.value.remove(item);
    })
    addData.forEach(item => {
        treeRef.value.append(item, node);
    })
}

const handleExpand = async (data: ResourceItem, node: Node) => {
    state.inputPath = data.path;
    defalultExpandedKeysSet.add(data.path)
    state.defalultExpandedKeys = [...defalultExpandedKeysSet];
    updateData(data.path,node);
}

const hanldeCollapse = (data: ResourceItem, node: Node) => {
    state.inputPath = data.path;
    defalultExpandedKeysSet.delete(data.path)
    state.defalultExpandedKeys = [...defalultExpandedKeysSet];
}

const handleClick = async (data: ResourceItem, node: Node) => {
    const oldData = resourceState.resourceTotal[data.path];
    if (oldData && oldData.length <= 0) {
        defalultExpandedKeysSet.add(data.path)
        state.defalultExpandedKeys = [...defalultExpandedKeysSet];
    }
    state.inputPath = data.path;
    updateData(data.path, node);
}

onMounted(() => {
    loadFiles(resourceState.currentPath)
})

const doSetCurrentPath = () => {
    const currentkey = treeRef.value.getCurrentKey();
    if (currentkey && (currentkey === state.inputPath || `${currentkey}\\` === state.inputPath)) {
        setCurrentPath(currentkey);
        const dataSource = { remote: LOCAL_HOST, port: PORT, dataPath: [currentkey] };
        confirm(dataSource);
        return true;
    } else {
        return false;
    }
}

const getLoadData = async (node: Node, resolve: (data: ResourceItem[]) => void) => {
    const resource = resourceState.startResource;
    if (node.level === 0) {
        return resolve(resource);
    }
    const path = node.data.path;
    const newData = await loadFiles(path);
    resolve(newData || []);
}

defineExpose({
    doSetCurrentPath
})
</script>

<template>
    <div class="tree-wrap">
        <el-input v-model="state.inputPath" disabled />
        <div class="data-tree">
            <el-tree ref="treeRef" :default-expanded-keys="state.defalultExpandedKeys" :data="resourceState.startResource" :props="defaultProps" :auto-expand-parent="false"
                node-key="path" @node-click="handleClick" @node-expand="handleExpand" @node-collapse="hanldeCollapse" lazy
                :load="getLoadData">
                <template #default="{ node, data }">
                    <div class="custom-tree-node">
                        <el-icon :size="16">
                            <FileIcon v-if="data.leaf" />
                            <FolderIcon v-else />
                        </el-icon>
                        <span :id="data.path">{{ data.name }}</span>
                    </div>
                </template>
            </el-tree>
        </div>
    </div>
</template>

<style scoped>
.tree-wrap {}

.data-path {
    border: 1px solid #656565;
    border-radius: 2px;
    font-size: 12px;
    color: var(--dataPath-color);
    line-height: 14px;
    font-weight: 400;
    height: 32px;
    padding: 8px 16px;
}

.el-tree {
  --el-tree-node-hover-bg-color: var(--color-border-hover);
  color: var(--dataPath-color) !important;
}

.data-tree {
    margin-top: 24px;
    height: 310px;
    overflow-y: auto;
    background: var(--dataTree-background);
    color: var(--dataPath-color) !important;
    padding: 10px;
    border-radius: 4px;
}

:deep(.el-input__wrapper) {
    border-radius: 2px;
    border: 1px solid #656565;
    box-shadow: 0 0 0;
    background-color: transparent !important;
}

:deep(.el-input.is-disabled .el-input__inner) {
    font-size: 12px;
    color: var(--dataTree-background);
    -webkit-text-fill-color: var(--dataPath-color);
    font-weight: 400;
    cursor: auto;
}

.custom-tree-node {
    flex: 1 1 auto;
    display: flex;
    align-items: center;
}

.custom-tree-node span {
    padding-left: 4px;
}

:deep(.el-tree-node.is-current > .el-tree-node__content) {
    background-color: var(--dataTree-onclick);
}
</style>