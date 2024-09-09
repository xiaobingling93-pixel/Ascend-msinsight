<script setup lang="ts">
import { ref } from 'vue';
import ImportIcon from '@/components/icons/import_icon.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import { useDataSources } from '@/stores/dataSource';
import ProjectMode from '@/components/ProjectMode.vue';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import ResourceDialog from '@/components/ResourceDialog.vue';
import { useSession } from '@/stores/session';
const { session } = useSession();

const showModal = ref(false);
const store = useDataSources();
const [ImportData] = useWatchTranslation(['Import Data']);

function addRemote(e: MouseEvent) {
    showModal.value = true;
}
</script>

<template>
    <header class="header">
        <ProjectMode />
        <ResourceDialog v-model:showModal="showModal" project-name=""></ResourceDialog>
    </header>
    <div class="container">
        <div class="btn-import" @click="addRemote">
            <el-icon class="icon-button" :size="16">
                <ImportIcon />
            </el-icon>
            <span>{{ ImportData }}</span>
        </div>
        <MenuTree :dataSource="store.menuTree"></MenuTree>
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


.btn-import{
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 8px;
    margin: 8px;
    background: var(--mi-bg-color-common);
    border-radius: var(--mi-border-radius-small);
    cursor: pointer;
    font-size: 12px;
}

.btn-import:hover{
    color: var(--mi-color-primary);
    transition: .3s;
}

.icon-button {
    margin-right: 8px;
}

.container{
    position: relative;
}
</style>