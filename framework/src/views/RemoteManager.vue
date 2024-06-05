<script setup lang="ts">
import { onMounted, ref, watch } from 'vue';
import AddIcon from '@/components/icons/cross_icon.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import { useDataSources } from '@/stores/dataSource';
import connector from '@/connection';
import ProjectMode from '@/components/ProjectMode.vue';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import ResourceDialog from '@/components/ResourceDialog.vue';

const isDarkTheme = ref(true);

onMounted(() => {
    connector.addListener('getParseStatus', () => {
      connector.send({
        event: 'setTheme',
        body: { isDark: isDarkTheme.value },
      });
    });
});
watch(isDarkTheme, () => {
    connector.send({
        event: 'setTheme',
        body: { isDark: isDarkTheme.value },
    });
    document.body.className = document.body.className === 'dark-theme' ? 'light-theme' : 'dark-theme';
});
const [ImportData, SwitchTheme] = useWatchTranslation(['Import Data','Switch Theme']);

const showModal = ref(false);

function addRemote(e: MouseEvent) {
    e.stopPropagation();
    showModal.value = true;
}

const store = useDataSources();

</script>

<template>
    <header class="header">
        <ProjectMode />
        <el-tooltip :content="ImportData" :effect="isDarkTheme ? 'light' : 'dark'">
          <el-icon class="icon-button" :size="16" @click="addRemote">
            <AddIcon />
          </el-icon>
        </el-tooltip>
        <el-tooltip :content="SwitchTheme" :effect="isDarkTheme ? 'light' : 'dark'">
          <el-switch class="theme-toggle" v-model="isDarkTheme"></el-switch>
        </el-tooltip>
        <ResourceDialog v-model:showModal=showModal project-name=""></ResourceDialog>
    </header>
    <div class="container">
        <MenuTree :dataSource="store.menuTree" :is-dark-theme="isDarkTheme"></MenuTree>
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
    background-color: var(--color-background);
}

.icon-button {
    border-radius: var(--border-radius);
}

.icon-button:hover {
    cursor: pointer;
}

.header .icon-container {
    margin-right: 0.5rem;
}
</style>