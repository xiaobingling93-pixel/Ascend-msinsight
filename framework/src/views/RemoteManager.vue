<script setup lang="ts">
import { ref } from 'vue';
import DeleteIcon from '@/components/icons/bin_icon.vue';
import ImportIcon from '@/components/icons/import_icon.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import { useDataSources } from '@/stores/dataSource';
import ProjectMode from '@/components/ProjectMode.vue';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import ResourceDialog from '@/components/ResourceDialog.vue';
import { ElMessageBox } from 'element-plus';
import { storeToRefs } from 'pinia';
import i18n from '@/i18n';

const showModal = ref(false);
const dataSourcesStore = useDataSources();
const { menuTree } = storeToRefs(dataSourcesStore);
const { removeAllProjects } = dataSourcesStore;
const [ImportData, DeleteAll, Cancel, Confirm] = useWatchTranslation(['Import Data', 'Delete All', 'Cancel', 'Confirm']);

function addRemote(e: MouseEvent) {
    showModal.value = true;
}

async function handleAllDelete() {
    await removeAllProjects();
}

const deleteAll = () => {
    if (menuTree.value.length === 0) {
        return;
    }
    ElMessageBox.confirm(i18n.t('DeleteAllConfirmDescribe'), {
        title: DeleteAll.value,
        confirmButtonText: Confirm.value,
        cancelButtonText: Cancel.value,
    }).then(() => {
        handleAllDelete();
    });
};
</script>

<template>
    <header class="header">
        <ProjectMode />
        <ResourceDialog v-model:showModal="showModal" project-name=""></ResourceDialog>
    </header>
    <div class="container">
        <div class="btn-box">
            <el-tooltip :content="DeleteAll" effect="light" :show-after="400">
                <div :class="['btn-item', 'btn-delete', { disabled: menuTree.length === 0 }]" @click="deleteAll">
                    <el-icon :size="16">
                        <DeleteIcon />
                    </el-icon>
                </div>
            </el-tooltip>

            <div class="btn-item btn-import" @click="addRemote">
                <el-icon class="icon-button" :size="16">
                    <ImportIcon />
                </el-icon>
                <span>{{ ImportData }}</span>
            </div>
        </div>
        <MenuTree :dataSource="menuTree"></MenuTree>
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

.btn-delete:not(.disabled):hover {
    color: var(--mi-color-danger);
    transition: 0.3s;
}

.btn-delete.disabled {
    cursor: not-allowed;
    color: var(--mi-text-color-disabled);
}

.icon-button {
    margin-right: 8px;
}

.container {
    position: relative;
}
</style>
