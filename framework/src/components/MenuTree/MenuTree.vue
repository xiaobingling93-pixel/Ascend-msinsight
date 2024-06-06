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


const [DeleteAll, DeleteItem, Cancel, Confirm] = useWatchTranslation(['Delete All', 'Delete Item', 'Cancel', 'Confirm']);
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
                        <el-tooltip :content="node.label" :effect="isDarkTheme ? 'light' : 'dark'">
                            <EditableText  v-if="node.level === 1" :tree-node="node" :key="data.id + data.label"></EditableText>
                            <span v-else class="contentNodeText">{{ node.label }} </span>
                        </el-tooltip>
                    </span>

                    <div class="btn-box">
                      <el-tooltip v-if="node.level === 1" content="Import Data" :effect="isDarkTheme ? 'light' : 'dark'">
                          <el-icon class="icon-button" @click.stop="addRemoteUnderProject(node, $event)">
                            <AddIcon />
                          </el-icon>
                      </el-tooltip>

                      <el-tooltip :content="node.level === 1 ? DeleteAll : DeleteItem" :effect="isDarkTheme ? 'light' : 'dark'">
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

.el-tree {
    --el-tree-node-hover-bg-color: var(--color-border-hover);
    color: var(--dataPath-color) !important;
}
::v-deep(.activateNode) {
  background-color:var(--treeNode-onclick);
}

::v-deep(.contentNode) {
    position: relative;
    display: flex;
    user-select: none;
    justify-content: space-between;
    flex: 1;
    line-height: 20px;
    width: calc(100% - 18px);
}

::v-deep(.contentBody) {
    display: flex;
    align-items: center;
    width: 0;
    flex: 1 1 auto;
    overflow: hidden;
    text-overflow: ellipsis;
}

::v-deep(.contentNodeText) {
    padding: 0 0 0 2px;
    font-size: 12px;
    color: var(--treeNode-color);
    font-weight: 400;
    overflow: hidden;
    white-space: nowrap;
}


::v-deep(.el-popper.is-dark) {
    color: red;
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
