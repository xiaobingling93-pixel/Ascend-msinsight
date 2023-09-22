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
}

const state = reactive({
    selectedPath: "",
    defalultExpandedKeys: [] as string[],
})

const handleExpand = (data: ResourceItem, node: Node) => {
    if (state.selectedPath === data.path) {
        return;
    }
    state.selectedPath = data.path;
    loadFiles(data.path);
}

const hanldeCollapse = (data: ResourceItem, node: Node) => {
    state.selectedPath = data.path;
    loadFiles(data.path);
}

const handleClick = (data: ResourceItem, node: Node) => {
    state.selectedPath = data.path;
}

const doOpenCurrent = (currentKey: string) => {
    if (treeRef.value) {
        treeRef.value.setCurrentKey(currentKey);
    } else {
        nextTick(() => {
            doOpenCurrent(currentKey)
        })
    }
}

onMounted(() => {
    loadFiles(resourceState.currentPath)
})

watch(() => resourceState.currentPath, (newVal) => {
    state.selectedPath = newVal;
    doOpenCurrent(newVal)

}, { immediate: true })

const doSetCurrentPath = () => {
    setCurrentPath(state.selectedPath);
    const dataSource = { remote: LOCAL_HOST, port: PORT, dataPath: [state.selectedPath] };
    confirm(dataSource)
}

defineExpose({
    doSetCurrentPath
})
</script>

<template>
    <div class="tree-wrap">
        <el-input v-model="state.selectedPath" disabled />
        <div class="data-tree">
            <el-tree ref="treeRef" :data="resourceState.resource" :props="defaultProps" :accordion="false" node-key="path"
                @node-click="handleClick" @node-expand="handleExpand" @node-collapse="hanldeCollapse">
                <template #default="{ node, data }">
                    <div class="custom-tree-node">
                        <el-icon :size="16">
                            <FolderIcon v-if="data.children" />
                            <FileIcon v-else />
                        </el-icon>
                        <span>{{ data.name }}</span>
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