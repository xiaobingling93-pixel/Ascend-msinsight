<script setup lang="ts">
import type { TreeNodeType } from './types';
import type Node from 'element-plus/es/components/tree/src/model/node';
import LocalIcon from '@/components/icons/loaclImport_icon.vue';
import DeletePopConfirm from '@/components/MenuTree/DeletePopConfirm.vue';
import EditableText from '@/components/MenuTree/EditableText.vue';
import AddLightIcon from '@/components/icons/add_light_icon.vue';
import AddDarkIcon from '@/components/icons/add_dark_icon.vue';
import { computed, ref, watch } from 'vue';
import { useDataSources } from '@/stores/dataSource';
import { LOCAL_HOST, PORT, ProjectActionEnum, type DataSource } from '@/centralServer/websocket/defs';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import ResourceDialog from '@/components/ResourceDialog.vue';
import { useSession } from '@/stores/session';
import { useCompareConfig } from '@/stores/compareConfig';
import ContextMenu from '@/components/ContextMenu.vue';
import { storeToRefs } from 'pinia';
import DeleteIcon from '@/components/icons/bin_icon.vue';
import {ElMessageBox} from 'element-plus';
import i18n from '@/i18n';
import {HandleSingleDoubleClick} from '@/utils';
import {MenuLevel} from '@/utils/enmus';
import type {MenuItem} from './types';

interface TreeData {
    id: string;
    label: string;
    projectName: string;
    filePath:string;
}

const { session } = useSession();
const dataSourcesStore = useDataSources();
const { confirm, removeProjects } = dataSourcesStore;
const { lastDataSource } = storeToRefs(dataSourcesStore);
const tree = ref();
const checkAll = ref(true);
const showModal = ref(false);
const projectName = ref('');

const compareConfigStore = useCompareConfig();
const { setBaselineData, setCompareData, cancelBaselineData, cancelCompareData } = compareConfigStore;
const { baselineDataInfo, compareDataInfo } = storeToRefs(compareConfigStore);

const selectProjectExplorerInfo = ref({ projectName: '', fileName: '' });

const collapsedKeys = ref<Set<string>>(new Set());
const allKeys = computed(() => props.dataSource.map(item => item.id));
const expandedKeys = computed(() => allKeys.value.filter(item => !collapsedKeys.value.has(item)));

const menuItems = computed(() => {
    const {projectName: selectedProjectName, fileName: selectedFileName} = selectProjectExplorerInfo.value;
    const {projectName: baselineProjectName, filePath: baselineFilePath} = baselineDataInfo.value;
    const {projectName: compareProjectName, filePath: compareFilePath} = compareDataInfo.value;
    const isProject = selectedProjectName !== '' && selectedFileName === '';
    const isBaseline = selectedProjectName === baselineProjectName && selectedFileName === baselineFilePath;
    const isBaselineSetted = baselineFilePath != '';
    const isComparison = selectedProjectName === compareProjectName && selectedFileName === compareFilePath;
    const allMenuItems: MenuItem[] = [
      {
        label: 'Set as Baseline Data',
        key: 'setAsBaselineData',
        action: (): void => {
          setBaselineData(selectProjectExplorerInfo.value.projectName, selectProjectExplorerInfo.value.fileName);
        },
        visible: !isBaseline,
      },
      {
        label: 'Unset as Baseline Data',
        key: 'unsetAsBaselineData',
        action: cancelBaselineData,
        visible: isBaseline
      },
      {
        label: 'Set as Comparison Data',
        key: 'setAsComparisonData',
        action: (): void => {
          setCompareData(selectProjectExplorerInfo.value.projectName, selectProjectExplorerInfo.value.fileName);
        },
        visible: !isProject && !isBaseline && !isComparison && isBaselineSetted
      },
      {
        label: 'Unset as Comparison Data',
        key: 'unsetAsComparisonData',
        action: cancelCompareData,
        visible: isComparison
      },
    ];
    return allMenuItems.filter(item => item.visible);
});

const [DeleteAll, DeleteItem, ImportData, Cancel, Confirm, All] = useWatchTranslation(['Delete All', 'Delete Item', 'Import Data', 'Cancel', 'Confirm', 'All']);
const props = defineProps<{
    dataSource: TreeNodeType[];
    editStatus: boolean;
}>();

const activateNode = computed(() => ({ projectName: lastDataSource.value.projectName, filePath: lastDataSource.value.dataPath[0] }));

const isActiveNode = (node: Node, data: TreeData): boolean => {
    if (node.level === MenuLevel.PROJECT) {
        return data.label === activateNode.value.projectName;
    }
    return data.label === activateNode.value.filePath && data.projectName === activateNode.value.projectName;
};

const handleNodeExpand = (data: TreeData) => {
  collapsedKeys.value.delete(data.id);
};

const handleNodeCollapse = (data: TreeData) => {
  collapsedKeys.value.add(data.id);
};

const handleNodeClick = (data: any, node: any) => {
    let dataSource = { remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] } as DataSource;
    if (node.level === MenuLevel.PROJECT) {
        if(lastDataSource.value.projectName === data.label) { return }
        const firstDataPath = data.children?.[0].label;
        dataSource.projectName = data.label;
        if(firstDataPath) {
            dataSource.dataPath.push(firstDataPath);
        }
    } else {
        dataSource.projectName = data.projectName;
        dataSource.dataPath.push(data.label);
    }
    confirm(dataSource, false, ProjectActionEnum.TRANSFER_PROJECT);
};

const handleSingleClick = (data: any, node: any) => {
  // Vue不区分单击、双击
  // 如果点击项目名，为避免影响双击事件，增加了额外控制
  if (node.level === MenuLevel.PROJECT) {
    HandleSingleDoubleClick.click(() => {
      handleNodeClick(data, node);
    }, 'projectName');
  } else {
    handleNodeClick(data, node);
  }
};

function addRemoteUnderProject(node: any, e: MouseEvent) {
    e.stopPropagation();
    projectName.value = node.data.label;
    showModal.value = true;
}

const handleRightClick = (data: any) => {
  selectProjectExplorerInfo.value.projectName = data.projectName ?? '';
  selectProjectExplorerInfo.value.fileName = data.filePath ?? '';
};

const getToolTip = (data:any, node: any): string => {
  if (node.level === 1) {
    return data.label;
  }
  const rankId = useCompareConfig().getRankIdByProjectInfo(data.projectName, data.label);
  if (rankId === '') {
    return data.label;
  }
  return rankId + ' : ' + data.label;
};

const customNodeClass = (data: TreeData,node:Node) => {
    const {projectName: baselineProjectName, filePath: baselineFilePath} = baselineDataInfo.value;
    const isBaselineNode=(node.level === MenuLevel.FILE && data.label === baselineFilePath) ||
        (node.level === MenuLevel.PROJECT && data.label === baselineProjectName && baselineFilePath === '');
    if (isBaselineNode) {
      return 'is-baseline';
    } else if (data.label === compareDataInfo.value.filePath) {
      return 'is-comparison';
    } else {
      return null;
    }
};

// 进入管理模式，默认选中全部
watch(() => [props.editStatus], () => {
  tree.value.setCheckedKeys(allKeys.value);
  checkAll.value = true;
});
// 选中的项目
const checkedProjectKeys = computed((): string[] => tree.value.getCheckedKeys().filter((key: string) => allKeys.value.includes(key)));

// 全选、取消全选
const handleCheckAllChange = (val: boolean) => {
  if (val) {
    tree.value.setCheckedKeys(allKeys.value);
  } else {
    tree.value.setCheckedKeys([]);
  }
};

// 更新全选状态
const handleCheckChange = () => {
  checkAll.value = checkedProjectKeys.value.length > 0 && checkedProjectKeys.value.length === allKeys.value.length;
};

// 删除项目
const handleDeleteChecked = () => {
  ElMessageBox.confirm(i18n.t('DeleteConfirmDescribe'), {
    title: DeleteItem.value,
    confirmButtonText: Confirm.value,
    cancelButtonText: Cancel.value,
  }).then(async () => {
    await removeProjects(checkedProjectKeys.value);
    // 抵消组件自动选中
    tree.value.setCheckedKeys([]);
  });
};

</script>
<template>
    <ContextMenu :menu-items="menuItems">
      <div class="manage-box" v-if="editStatus">
        <el-checkbox v-model="checkAll" @change="handleCheckAllChange">{{ All }}</el-checkbox>
        <div :class="['btn-item', 'btn-delete', { disabled: checkedProjectKeys.length === 0 }]" @click="handleDeleteChecked">
          <el-icon :size="16">
            <DeleteIcon />
          </el-icon>
        </div>
      </div>
        <div class="menu-tree">
            <el-tree
                :data="props.dataSource"
                node-key="id"
                :default-expanded-keys="expandedKeys"
                :expand-on-click-node="false"
                :props="{ class: customNodeClass }"
                @node-expand="handleNodeExpand"
                @node-collapse="handleNodeCollapse"
                @node-click="handleSingleClick"
                :show-checkbox="props.editStatus"
                @check-change="handleCheckChange"
                ref="tree"
            >
                <template #default="{ node, data }">
                    <div :class="['content-node', { 'activate-node': isActiveNode(node, data) }]">
                        <span class="content-body">
                            <LocalIcon v-if="node.level === 1" style="flex: none" />
                            <el-tooltip :content="getToolTip(data, node)" effect="light" :show-after="400">
                                <EditableText v-if="node.level === 1" :tree-node="node" :key="data.id + data.label"
                                              @contextmenu.prevent="handleRightClick(data)"></EditableText>
                                <span v-else class="content-node-text can-right-click" @contextmenu.prevent="handleRightClick(data)">
                                    {{ getToolTip(data, node) }}
                                </span>
                            </el-tooltip>
                        </span>

                        <div class="btn-box">
                            <el-tooltip v-if="node.level === 1" :content="ImportData" effect="light">
                                <el-icon class="icon-button" @click.stop="addRemoteUnderProject(node, $event)">
                                    <AddDarkIcon v-if="session.theme == 'dark'" />
                                    <AddLightIcon v-else />
                                </el-icon>
                            </el-tooltip>

                            <el-tooltip :content="node.level === 1 ? DeleteAll : DeleteItem" effect="light" :show-after="400">
                                <div class="icon-button">
                                    <DeletePopConfirm :data="data" :node="node" :is-delete-all="node.level === 1" />
                                </div>
                            </el-tooltip>
                        </div>
                    </div>
                </template>
            </el-tree>
            <ResourceDialog v-model:showModal="showModal" :project-name="projectName"></ResourceDialog>
        </div>
    </ContextMenu>
</template>

<style scoped>
.menu-tree {
    padding: 0.5rem 0.8rem;
}

.menu-tree .el-tree{
    --el-tree-node-hover-bg-color: var(--mi-tree-bg-color-light);
}

.content-node {
    position: relative;
    display: flex;
    user-select: none;
    justify-content: space-between;
    flex: 1;
    line-height: 20px;
    width: calc(100% - 18px);
}

.content-node:hover .btn-box,
.content-node.activate-node .btn-box {
    display: flex;
}

.content-body {
    display: flex;
    align-items: center;
    width: 0;
    flex: 1 1 auto;
}

.content-node-text {
    padding: 0 0 0 2px;
    font-size: 12px;
    color: var(--tree-child-node-text-color);
    font-weight: 400;
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
}

:deep(.el-tree-node__children .el-tree-node:focus > .el-tree-node__content:not(:has(.activate-node))) {
    background-color: transparent;
}

:deep(.el-tree-node__content:has(.activate-node)) {
    background-color: var(--mi-tree-bg-color-light);
}

.btn-box {
    display: none;
    align-items: center;
    justify-content: end;
    width: 40px;
    margin-left: 10px;
}
.icon-button {
    margin-left: 4px;
}

.is-baseline > .el-tree-node__content .content-node-text {
    font-weight: bold;
    color: var(--mi-color-primary);
}
.is-comparison > .el-tree-node__content .content-node-text {
    font-weight: bold;
    color: var(--mi-color-warning);
}

.manage-box {
  padding: 0 8px;
  display: flex;
  align-items: end;
  justify-content: space-between;
}

.btn-delete:not(.disabled):hover {
  color: var(--mi-color-danger);
  transition: 0.3s;
  cursor: pointer;
}

.btn-delete.disabled {
  cursor: default;
  pointer-events: none;
  color: var(--mi-text-color-disabled);
}

.btn-item {
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 4px;
  background: var(--mi-bg-color-common);
  border-radius: var(--mi-border-radius-small);
  cursor: pointer;
  font-size: 12px;
}
</style>
