<script setup lang="ts">
import { ref, watch } from 'vue';
import AddIcon from '@/components/icons/cross_icon.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import ResourceComp from '@/components/ResourceComp.vue';
import { useDataSources } from '@/stores/dataSource';
import connector from '@/connection';
import ProjectMode from "@/components/ProjectMode.vue";
const isDarkTheme = ref(true);

const resourceComp = ref();

watch(isDarkTheme, () => {
    connector.send({ body: { event: 'setTheme', body: { isDark: isDarkTheme.value } } });
});

const showModal = ref(false);
function addRemote(e: MouseEvent) {
    e.stopPropagation();
    showModal.value = true;
}

const store = useDataSources();

const handleConfirm = () => {
    resourceComp.value.doSetCurrentPath();
    showModal.value = false;
}
</script>

<template>
    <header class="header">
        <ProjectMode />
        <el-icon class="icon-button" :size="16" @click="addRemote">
            <AddIcon />
        </el-icon>
        <el-switch class="theme-toggle" v-model="isDarkTheme"></el-switch>
        <el-dialog v-model="showModal" title="File Explorer" width="30%" destroy-on-close>
            <ResourceComp ref="resourceComp" />
            <template #footer>
                <span>
                    <el-button type="primary" @click="handleConfirm">Confirm</el-button>
                    <el-button @click="showModal = false">Cancel</el-button>
                </span>
            </template>
        </el-dialog>
    </header>
    <div class="container">
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
    background-color: var(--header-background-color);
}

.theme-toggle {
    --el-switch-off-color: rgb(141, 141, 141);
    --el-switch-on-color: rgb(60, 60, 60);
}

.container {
    flex-grow: 1;
    background-color: #252526;
}

.icon-button {
    border-radius: var(--border-radius);
}

.icon-button:hover {
    border: var(--border-style-hover);
    background-color: var(--color-background-soft);
}

.header .icon-container {
    margin-right: 0.5rem;
}
</style>