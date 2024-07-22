<script setup lang="ts">
import type { TreeNodeType } from './types';
import LocalIcon from '@/components/icons/loaclImport_icon.vue';
import DeletePopConfirm from '@/components/MenuTree/DeletePopConfirm.vue';
import EditableText from '@/components/MenuTree/EditableText.vue';
import AddIcon from '@/components/icons/cross_icon.vue';
import {ref, watch} from 'vue';
import {useDataSources} from '@/stores/dataSource';
import { LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import ResourceDialog from '@/components/ResourceDialog.vue';

const showModal = ref(false);
const projectName = ref('');
const activateNodeId = ref(0);


const [DeleteAll, DeleteItem, Cancel, Confirm, ImportData] = useWatchTranslation(['Delete All', 'Delete Item', 'Cancel', 'Confirm', 'Import Data']);
const props = defineProps<{
    dataSource: TreeNodeType[];
    isDarkTheme: boolean;
}>();

watch(
    () => [useDataSources().lastDataSource, props.dataSource],
    ()=>{
      const idx = props.dataSource.findIndex((item) => item.label === useDataSources().lastDataSource?.projectName);
      if (idx === -1) {
        activateNodeId.value = -1;
        return;
      }
      const project = props.dataSource[idx];
      activateNodeId.value = project.id;
    }
);

const handleNodeClick = (data:any, node: any) => {
  if (node.level === 1) {
    const dataSource = { remote: LOCAL_HOST, port: PORT, projectName: data.label, dataPath: [] };
    useDataSources().confirm(dataSource, false);
  }
};

function addRemoteUnderProject(node:any, e: MouseEvent) {
  e.stopPropagation();
  projectName.value = node.data.label;
  showModal.value = true;
}

</script>
<template>
    <div class="menu-tree">
        <el-tree :data="props.dataSource" node-key="id" :default-expand-all="true" :expand-on-click-node="false" :current-node-key="2" @node-click="handleNodeClick">
            <template #default="{ node, data }">
                <div :class="['contentNode', {activateNode:data.id === activateNodeId}]">
                    <span class="contentBody">
                        <LocalIcon v-if="node.level === 1" style="flex: none"/>
                        <el-tooltip :content="node.label" effect="light" :show-after="200">
                            <EditableText  v-if="node.level === 1" :tree-node="node" :key="data.id + data.label"></EditableText>
                            <span v-else class="contentNodeText">{{ node.label }} </span>
                        </el-tooltip>
                    </span>

                    <div class="btn-box">
                      <el-tooltip v-if="node.level === 1" :content="ImportData" effect="light">
                          <el-icon class="icon-button" @click.stop="addRemoteUnderProject(node, $event)">
                            <AddIcon />
                          </el-icon>
                      </el-tooltip>

                      <el-tooltip :content="node.level === 1 ? DeleteAll : DeleteItem" effect="light">
                        <div class="icon-button">
                          <DeletePopConfirm :data="data" :node="node" :is-delete-all="node.level === 1"/>
                        </div>
                      </el-tooltip>
                    </div>
                </div>
            </template>
        </el-tree>
        <ResourceDialog v-model:showModal=showModal :project-name="projectName"></ResourceDialog>
    </div>
</template>
<style scoped>
.menu-tree {
    padding: 0.5rem 0.8rem;
}


:deep(.el-tree-node__content:has(.activateNode)) {
  background-color: var(--mi-bg-color-light);
}

.contentNode {
    position: relative;
    display: flex;
    user-select: none;
    justify-content: space-between;
    flex: 1;
    line-height: 20px;
    width: calc(100% - 18px);
}

.contentBody {
    display: flex;
    align-items: center;
    width: 0;
    flex: 1 1 auto;
}

.contentNodeText {
    padding: 0 0 0 2px;
    font-size: 12px;
    color: var(--mi-text-color-tertiary);
    font-weight: 400;
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
}
:deep(.el-tree-node__children .el-tree-node:focus > .el-tree-node__content) {
    background-color: transparent;
}
:deep(.el-tree-node__children .el-tree-node__content:hover) {
    background-color: transparent;
}
.btn-box{
  display: flex;
  align-items: center;
  justify-content: end;
  width: 40px;
  margin-left: 10px;
}
.icon-button{
  margin-left: 4px;
}
</style>
