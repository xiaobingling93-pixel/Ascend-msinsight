<script setup lang="ts">
import { ref, reactive, watch, onMounted, nextTick } from 'vue';
import type Node from 'element-plus/es/components/tree/src/model/node';
import FolderIcon from '@/components/icons/folder_icon.vue';
import { useResource, type ResourceItem } from '@/stores/resourceComp';
import { addDataPath } from '@/centralServer/server';
import { LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';

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

const handleClick = (data: ResourceItem, node: Node) => {
    if (state.selectedPath === data.path) {
        return;
    }
    state.selectedPath = data.path;
    loadFiles(data.path);
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
    addDataPath({ remote: LOCAL_HOST, port: PORT, dataPath: [state.selectedPath] })
}

defineExpose({
    doSetCurrentPath
})
</script>

<template>
    <div class="tree-wrap">
        <!-- <p class="data-path">{{ resourceState.currentPath }}</p> -->
        <el-input v-model="state.selectedPath" disabled />
        <div class="data-tree">
            <el-tree ref="treeRef" :data="resourceState.resource" :props="defaultProps" :accordion="false" node-key="path"
                @node-click="handleClick">
                <template #default="{ node, data }">
                    <div class="custom-tree-node">
                        <FolderIcon v-if="data.children" />
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
    color: #F4F6FA;
    line-height: 14px;
    font-weight: 400;
    height: 32px;
    padding: 8px 16px;
}

.data-tree {
    margin-top: 24px;
    height: 310px;
    overflow-y: auto;
    background: #0a0a0a;
    padding: 10px;
    border-radius: 4px;
}

:deep(.el-input__wrapper) {
    border-radius: 2px;
    border: 1px solid #656565;
    box-shadow: 0 0 0;
}

:deep(.el-input.is-disabled .el-input__inner) {
    font-size: 12px;
    color: #F4F6FA;
    -webkit-text-fill-color: #F4F6FA;
    font-weight: 400;
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
    background-color: #2c2c2c;
}
</style>