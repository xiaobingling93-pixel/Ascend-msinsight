<script setup lang="ts">
import { ref, reactive, watch, onMounted, nextTick } from 'vue';
import type Node from 'element-plus/es/components/tree/src/model/node';
import FolderIcon from '@/components/icons/folder_icon.vue';
import FileIcon from '@/components/icons/file_icon.vue';
import { useResource, type ResourceItem } from '@/stores/resourceComp';
import { LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import { useDataSources } from '@/stores/dataSource';

const { confirm } = useDataSources();

const treeRef = ref();

const { resourceState, loadFiles, setCurrentPath, fileExist } = useResource();

const defaultProps = {
    label: 'name',
    children: 'children',
    isLeaf: 'leaf',
}
let isUpdateLoading = {} as {[key: string]: boolean};
let defalultExpandedKeysSet: Set<string> = new Set();

const state = reactive({
    defalultExpandedKeys: [] as string[],
    inputPath: '',
})

const updateData = async (path: string, node: Node) => {
    if (isUpdateLoading[path]) {
        return;
    }
    isUpdateLoading[path] = true;
    const oldData = resourceState.resourceTotal[path];
    if (oldData) {
        const newData = await loadFiles(path);
        checkData(newData, oldData, node)
    }
    isUpdateLoading[path] = false;
}

const checkData = (newData: ResourceItem[], oldData: ResourceItem[], node: Node) => {
    const newPaths = newData.map(item => item.path);
    const oldPaths = oldData.map(item => item.path);
    const deleteData = oldData.filter(item => !newPaths.includes(item.path));
    deleteData.forEach(item => {
        treeRef.value.remove(item);
    })
    newData.forEach((item, i) => {
        if (!oldPaths.includes(item.path)) {
            let findNext = false;
            for (let next = i + 1; next < newData.length; next++) {
                const nextPath = newData[next].path;
                const nextNode = treeRef.value.getNode(nextPath);
                if (nextNode) {
                    treeRef.value.insertBefore(item, nextNode);
                    findNext = true;
                    break;
                }
            }
            if (!findNext) {
                for (let pre = i - 1; pre >= 0; pre--) {
                    const prePath = newData[pre].path;
                    const preNode = treeRef.value.getNode(prePath);
                    if (preNode) {
                        treeRef.value.insertAfter(item, preNode);
                        findNext = true;
                        break;
                    }
                }
            }
            if (!findNext) {
                treeRef.value.append(item, node);
            }
        }
    })
}

const handleExpand = async (data: ResourceItem, node: Node) => {
    state.inputPath = data.path;
    defalultExpandedKeysSet.add(data.path)
    state.defalultExpandedKeys = [...defalultExpandedKeysSet];
    updateData(data.path,node);
    props.changeConfirmButtonState(true);
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
    props.changeConfirmButtonState(true);
}

let findFile = false;
let errorAlert = ref(false);

const searchPath = async () => {
  if (!state.inputPath) {
    props.changeConfirmButtonState(false);
    findFile = false;
    return;
  }
  const exist = await fileExist(state.inputPath);
  if (!exist) {
    props.changeConfirmButtonState(false);
    findFile = false;
    errorAlert.value = true;
    for (let i = 0; i < treeRef.value.store._getAllNodes().length; i++) {
      treeRef.value.store._getAllNodes()[i].expanded = false;
    }
    return;
  }
  if (findFile) {
    return;
  }
  if (exist && treeRef.value.getCurrentKey !== state.inputPath) {
    treeRef.value.filter(state.inputPath);
    if (!findFile) {
      await searchPath();
    }
  }
  findFile = false;
}


const filterNode = async (value: string, data: ResourceItem, node: Node): Promise<boolean> => {
  if (!value) {
    props.changeConfirmButtonState(false);
    state.inputPath = value;
    findFile = false;
    return true;
  }
  if (value === data.path) {
    await handleExpand(data, node);
    await handleClick(data, node);
    state.inputPath = value;
    treeRef.value.setCurrentKey(value);
    props.changeConfirmButtonState(true);
    errorAlert.value = false;
    findFile = true;
    return true;
  }
  if (value.startsWith(data.path)) {
    await handleExpand(data, node);
    // 展开后需要把搜索内容改回原来的文字
    state.inputPath = value;
    return true;
  }
  props.changeConfirmButtonState(false);
  state.inputPath = value;
  findFile = false;
  return true;
}


function expandPath(defaultSelectedDir: string) {
  let paths = defaultSelectedDir.split('/');

  treeRef.value.getNode('/').expand(() => {
    treeRef.value.getNode('/' + paths[1]).expand(() => {
      treeRef.value.getNode('/' + paths[1] + '/' + paths[2]).expand(() => {
        let currentNode = treeRef.value.getNode(defaultSelectedDir);
        currentNode.expand();

        treeRef.value.setCurrentKey(defaultSelectedDir);
        state.inputPath = defaultSelectedDir;
      });
    });
  });
}

onMounted(() => {
    loadFiles(resourceState.currentPath);
  if (!window.location.pathname.includes('\/proxy\/')) {
    loadFiles(resourceState.currentPath);
  } else {
    // 云指定路径
    let defaultSelectedDir = '/home/ma-user/work';

    loadFiles(resourceState.currentPath).then(() => {
      expandPath(defaultSelectedDir);
    })
  }
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
};

const getLoadData = async (node: Node, resolve: (data: ResourceItem[]) => void) => {
    const resource = resourceState.startResource;
    if (node.level === 0) {
        return resolve(resource);
    }
    const path = node.data.path;
    const newData = await loadFiles(path);
    return resolve(newData || []);
}

const doWhileOpenDialog = () => {
    const node = treeRef.value.getCurrentNode();
    if (node) {
      updateData(node.path, node);
    }
}
const props = defineProps(['changeConfirmButtonState']);
defineExpose({
    doSetCurrentPath,
    doWhileOpenDialog,
})
</script>

<template>
    <div class="tree-wrap">
        <el-text v-if="errorAlert.valueOf()" type="danger"> File not found, check the file path</el-text>
        <el-text v-else type="primary"> Enter the file path and press enter to search</el-text>
        <el-input v-if="errorAlert.valueOf()" class= "red-input" v-model="state.inputPath" @keyup.enter="searchPath" />
        <el-input v-else class= "bule-input" v-model="state.inputPath" @keyup.enter="searchPath" />
        <div class="data-tree">
            <el-tree ref="treeRef" :default-expanded-keys="state.defalultExpandedKeys" :data="resourceState.startResource" :props="defaultProps" :auto-expand-parent="false"
                node-key="path" @node-click="handleClick" @node-expand="handleExpand" @node-collapse="hanldeCollapse" lazy
                :load="getLoadData" :filter-node-method = "filterNode">
                <template #default="{ node, data }">
                    <div class="custom-tree-node">
                        <el-icon :size="16">
                            <FileIcon v-if="data.leaf" />
                            <FolderIcon v-else />
                        </el-icon>
                        <span :id="data.path" :name="data.path.replaceAll(/\\|:/g,'').replaceAll(/\./g, '--dot--')">{{ data.name }}</span>
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
    box-shadow: 0 0 0;
    background-color: transparent !important;
}
.red-input {
    border-radius: 2px;
    border: 1px solid #F56C6C;
    box-shadow: 0 0 0;
    background-color: transparent !important;
    --el-input-focus-border-color: #F56C6C;
}

.bule-input {
  border-radius: 2px;
  border: 1px solid #409EFF;
  box-shadow: 0 0 0;
  background-color: transparent !important;
  --el-input-focus-border-color: #409EFF;
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
.el-tree{
  width: fit-content;
  min-width: 100%;
}
.el-tree-node{
  width: fit-content;
}
</style>