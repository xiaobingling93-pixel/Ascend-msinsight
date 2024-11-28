<script setup lang="ts">
import {ref, watch} from 'vue';
import SetIcon from '@/components/icons/set_icon.vue';
import ImportIcon from '@/components/icons/import_icon.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import { useDataSources } from '@/stores/dataSource';
import ProjectMode from '@/components/ProjectMode.vue';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import ResourceDialog from '@/components/ResourceDialog.vue';
import { storeToRefs } from 'pinia';

const showModal = ref(false);
const editStatus = ref(false);
const dataSourcesStore = useDataSources();
const { menuTree } = storeToRefs(dataSourcesStore);
const [ImportData, Cancel, Confirm] = useWatchTranslation(['Import Data', 'Cancel', 'Confirm']);

function addRemote(e: MouseEvent) {
    showModal.value = true;
}

// 进入/退出项目管理
const switchEditStatus = () => {
  if (menuTree.value.length === 0) {
    return;
  }
  editStatus.value = !editStatus.value;
};

// 项目数量为空时，退出管理状态
watch(() => [menuTree.value.length], () => {
  if (menuTree.value.length === 0) {
    editStatus.value = false;
  }
});

</script>

<template>
    <header class="header">
        <ProjectMode />
        <ResourceDialog v-model:showModal="showModal" project-name=""></ResourceDialog>
    </header>
    <div class="container">
        <div class="btn-box">
            <div :class="['btn-item', 'btn-import']" @click="addRemote">
                <el-icon class="icon-button" :size="16">
                    <ImportIcon />
                </el-icon>
                <span>{{ ImportData }}</span>
            </div>
            <div :class="['btn-item', 'btn-set', { disabled: menuTree.length === 0 }]" @click="switchEditStatus">
              <el-icon :size="16" v-if="!editStatus">
                <SetIcon/>
              </el-icon>
              <div v-else>{{ Cancel }}</div>
            </div>
        </div>
        <MenuTree :dataSource="menuTree" :editStatus="editStatus"></MenuTree>
    </div>
</template>

<style scoped>
header {
    display: flex;
    justify-content: end;
    gap: 7px;
    flex-direction: row;
    height: var(--header-height);
    align-items: center;
    min-width: 80px;
    padding: 0 15px 0 5px;
    background: var(--header-bg-color);
    border-bottom: 1px solid var(--mi-bg-color-light);
}

.btn-box {
    display: flex;
    align-items: center;
    padding: 8px;
    gap: 8px;
}

.btn-item {
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 8px;
    background: var(--mi-bg-color-common);
    border-radius: var(--mi-border-radius-small);
    cursor: pointer;
    font-size: 12px;
}

.btn-import {
    flex: 1;
}

.btn-import:hover {
    color: var(--mi-color-primary);
    transition: 0.3s;
}

.btn-item:not(.disabled):hover {
    color: var(--mi-color-primary);
    transition: 0.3s;
}

.btn-item.disabled {
    cursor: not-allowed;
    pointer-events: none;
    color: var(--mi-text-color-disabled);
}

.icon-button {
    margin-right: 8px;
}

.container {
    position: relative;
}
</style>
