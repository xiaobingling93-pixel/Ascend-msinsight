<script setup lang="ts">
import {ref, reactive, onMounted, computed, watch, nextTick} from 'vue';
import type Node from 'element-plus/es/components/tree/src/model/node';
import FolderIcon from '@/components/icons/folder_icon.vue';
import FileIcon from '@/components/icons/file_icon.vue';
import RefreshIcon from '@/components/icons/refresh.vue';
import { useResource, type ResourceItem } from '@/stores/resourceComp';
import {LOCAL_HOST, PORT, ProjectActionEnum} from '@/centralServer/websocket/defs';
import { useDataSources } from '@/stores/dataSource';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import { console } from '@/utils/console';
import { ProjectErrorType } from '@/utils/enmus';
import {LocalStorageKeys, localStorageService} from '@/utils/local-storage';

const props = defineProps<{ maxPathLen: number, changeConfirmButtonState: (arg0: boolean) => void, show: boolean }>();
const emit = defineEmits(['input-change']);

const { confirm, checkProjectValid } = useDataSources();

const treeRef = ref();

const { resourceState, loadFiles, setCurrentPath, fileExist } = useResource();

const defaultProps = {
    label: 'name',
    children: 'children',
    isLeaf: 'leaf',
};
let isUpdateLoading = {} as {[key: string]: boolean};
let defalultExpandedKeysSet: Set<string> = new Set();

const state = reactive({
    defalultExpandedKeys: [] as string[],
    inputPath: localStorageService.getItem(LocalStorageKeys.LAST_FILE_PATH),
});

watch(() => state.inputPath, () => {
  emit('input-change', state.inputPath.length);
});

watch(() => props.show, (value) => {
  if(value) {
    handleMounted();
  }
});

const updateData = async (path: string, node: Node) => {
    if (isUpdateLoading[path]) {
        return;
    }
    isUpdateLoading[path] = true;
    const oldData = resourceState.resourceTotal[path];
    if (oldData) {
        const newData = await loadFiles(path);
        checkData(newData, oldData, node);
    }
    isUpdateLoading[path] = false;
};

const checkData = (newData: ResourceItem[], oldData: ResourceItem[], node: Node) => {
    const newPaths = newData.map(item => item.path);
    const oldPaths = oldData.map(item => item.path);
    const deleteData = oldData.filter(item => !newPaths.includes(item.path));
    deleteData.forEach(item => {
        treeRef.value.remove(item);
    });
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
    });
};

const handleExpand = async (data: ResourceItem, node: Node) => {
    state.inputPath = data.path;
    defalultExpandedKeysSet.add(data.path);
    state.defalultExpandedKeys = [...defalultExpandedKeysSet];
    updateData(data.path,node);
    props.changeConfirmButtonState(true);
};

const hanldeCollapse = (data: ResourceItem, node: Node) => {
    state.inputPath = data.path;
    defalultExpandedKeysSet.delete(data.path);
    state.defalultExpandedKeys = [...defalultExpandedKeysSet];
};

const handleClick = async (data: ResourceItem, node: Node) => {
    const oldData = resourceState.resourceTotal[data.path];
    if (oldData && oldData.length <= 0) {
        defalultExpandedKeysSet.add(data.path);
        state.defalultExpandedKeys = [...defalultExpandedKeysSet];
    }
    state.inputPath = data.path;
    updateData(data.path, node);
    props.changeConfirmButtonState(true);
    errorAlert.value = false;
};

let findFile = false;
let errorAlert = ref(false);
const [FileSearchDescribe, FileNotFundDescribe, RefreshDirectory] = useWatchTranslation(['FileSearchDescribe', 'FileNotFundDescribe', 'RefreshDirectory']);
const inputErrorText = computed(() => errorAlert.value ? FileNotFundDescribe.value : FileSearchDescribe.value);

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
    const nodeEI = document.getElementById(state.inputPath);
    setTimeout(() => {
      if (nodeEI) {
        nextTick(() => {
          nodeEI.scrollIntoView({behavior: 'smooth', block: 'center'});
        });
      }
    },500);
    return;
  }
  if (exist && treeRef.value.getCurrentKey !== state.inputPath) {
    treeRef.value.filter(state.inputPath);
    if (!findFile) {
      await searchPath();
    }
  }
  findFile = false;
};


const filterNode = async (value: string, data: ResourceItem, node: Node): Promise<boolean> => {
  if (!value) {
    props.changeConfirmButtonState(false);
    state.inputPath = value;
    findFile = false;
    return true;
  }
  const realFilePath = removeTrailingSlashes(value);
  if (realFilePath === data.path) {
    await handleExpand(data, node);
    await handleClick(data, node);
    state.inputPath = realFilePath;
    treeRef.value.setCurrentKey(realFilePath);
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
};

function removeTrailingSlashes(str: string) {
  return str.replace(/[^\/\\][\/\\]+$/, (match: string) => match.substr(0,1));
}


function expandPath(defaultSelectedDir: string): void {
  const paths = defaultSelectedDir.split('/');
  treeRef.value.getNode('/').expand(() => {
    if(paths[1] === undefined) {
      return;
    }
    treeRef.value.getNode('/' + paths[1]).expand(() => {
      if(paths[2] === undefined) {
        return;
      }
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
  handleMounted();
});

const handleMounted = () => {
  if (!window.location.pathname.includes('\/proxy\/')) {
    loadFiles('');
  } else {
    // 云指定路径
    let defaultSelectedDir = '/home/ma-user/work';

    loadFiles('').then(() => {
      expandPath(defaultSelectedDir);
    });
  }

  expandLastSelectedFile();
  // 选中并滚动至当前节点
  setTimeout(() => {
    const node = treeRef.value.getNode(state.inputPath);
    if(node) {
      treeRef.value.setCurrentKey(node.key);
      nextTick(() => {
        const nodeEl = treeRef.value.el$.querySelector('.is-current > .el-tree-node__content');
        nodeEl?.scrollIntoView({behavior:'smooth',block: 'center'});
      });
    }
  }, 800);
};

// 每次打开弹窗只展开上一次选择的目录
const expandLastSelectedFile = () => {
    for (let path of defalultExpandedKeysSet) {
        if (!state.inputPath.startsWith(path)) {
            defalultExpandedKeysSet.delete(path);
        }
    }
    state.defalultExpandedKeys = [...defalultExpandedKeysSet];
};

const doCheckFileVallid = async (projectName: string) => {
    const currentkey = treeRef.value.getCurrentKey().replace(/[^\\/][\\/]+$/, (match: string) => match.substr(0,1));
    const curProjectName = projectName === '' ? currentkey : projectName;
    if (currentkey && currentkey === state.inputPath) {
        setCurrentPath(currentkey);
        const dataSource = { remote: LOCAL_HOST, port: PORT, projectName: curProjectName, dataPath: [currentkey] };
        try {
          return await checkProjectValid(dataSource);
        } catch {
          console.log('doCheckFileVallid error.');
          return ProjectErrorType.NO_ERRORS;
        }
    } else {
        return ProjectErrorType.NO_ERRORS;
    }
};

const doSetCurrentPath = (projectName: string, isConflict: boolean) => {
    const currentkey = treeRef.value.getCurrentKey().replace(/[^\\/][\\/]+$/, (match: string) => match.substr(0,1));
    const curProjectName = projectName === '' ? currentkey : projectName;
    if (currentkey && currentkey === state.inputPath) {
        setCurrentPath(currentkey);
        const dataSource = { remote: LOCAL_HOST, port: PORT, projectName: curProjectName, dataPath: [currentkey] };
        try {
          confirm(dataSource, isConflict, ProjectActionEnum.ADD_FILE);
          return { result: true };
        } catch {
          console.log('doSetCurrentPath error.');
          return { result: false, inputPath: state.inputPath };
        }
    } else {
        return { result: false, inputPath: state.inputPath };
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
};

const doWhileOpenDialog = () => {
    const node = treeRef.value.getCurrentNode();
    if (node) {
      updateData(node.path, node);
    }
};
defineExpose({
    doSetCurrentPath,
    doWhileOpenDialog,
    doCheckFileVallid,
});
</script>

<template>
    <div class="tree-wrap">
        <div class="input-wrapper">
          <el-form ref="formRef" class="form" :model="state" @submit.prevent="searchPath">
            <el-form-item prop="inputPath" :validate-status="errorAlert ? 'error' : 'success'">
              <el-input v-model="state.inputPath" size="default" :maxlength="props.maxPathLen" show-word-limit :placeholder="FileSearchDescribe">
                <template #suffix>
                  <el-tooltip :content="RefreshDirectory">
                    <el-icon :size="16" @click="handleMounted" class="icon-refresh"><RefreshIcon /></el-icon>
                  </el-tooltip>
                </template>
              </el-input>
            </el-form-item>
          </el-form>
        </div>
        <el-text size="default" :type="errorAlert ? 'danger' : 'primary'"> {{ inputErrorText }} </el-text>
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
                        <span :name="data.path?.replace(/\\|:|\//g,'_').replace(/\./g, '--dot--')" :id="data.path">{{ data.name }}</span>
                    </div>
                </template>
            </el-tree>
        </div>
    </div>
</template>

<style scoped>
.el-tree{
  width: fit-content;
  min-width: 100%;
}

:deep(.el-tree-node.is-current > .el-tree-node__content) {
  background-color: var(--mi-bg-color-light);
  color: var(--mi-text-color-primary)
}

.form{
  width: 100%;
}

.el-form-item--small{
  margin-bottom: 8px;
}

.data-tree {
    margin-top: 24px;
    height: 310px;
    overflow-y: auto;
    background: var(--mi-bg-color-dark);
    padding: 10px 0;
}

.custom-tree-node {
    flex: 1 1 auto;
    display: flex;
    align-items: center;
}

.custom-tree-node span {
    padding-left: 4px;
}

.input-wrapper{
  display: flex;
  align-items: center;
}

.icon-refresh{
  cursor: pointer;
}
.icon-refresh:hover .icon{
  fill: var(--el-color-primary)
}
</style>