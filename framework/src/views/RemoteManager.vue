<script setup lang="ts">
import {onMounted, ref} from 'vue';
import AddIcon from '@/components/icons/cross_icon.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import {useDataSources} from '@/stores/dataSource';
import connector from '@/connection';
import ProjectMode from '@/components/ProjectMode.vue';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import ResourceDialog from '@/components/ResourceDialog.vue';
import {LocalStorageKeys, localStorageService} from '@/utils/local-storage';

const isDarkTheme = ref(localStorageService.getItem(LocalStorageKeys.THEME) === 'dark');
const showModal = ref(false);
const store = useDataSources();

onMounted(() => {
    changeElementTheme(isDarkTheme.value);
    connector.addListener('getParseStatus', () => {
      connector.send({
        event: 'setTheme',
        body: { isDark: isDarkTheme.value },
      });
    });
});

function changeElementTheme(isDark: boolean) {
  if (isDark) {
    document.documentElement.classList.add('dark');
  } else {
    document.documentElement.classList.remove('dark');
  }
}
function handleThemeChange(isDark: boolean) {
    changeElementTheme(isDark);
    const theme = isDark ? 'dark' : 'light';
    localStorageService.setItem(LocalStorageKeys.THEME, theme);
    connector.send({
        event: 'setTheme',
        body: { isDark: isDark },
    });
    document.body.className = isDark ? 'dark-theme' : 'light-theme';
}
const [ImportData, SwitchTheme] = useWatchTranslation(['Import Data','Switch Theme']);

function addRemote(e: MouseEvent) {
    e.stopPropagation();
    showModal.value = true;
}

</script>

<template>
    <header class="header">
        <ProjectMode />
        <el-tooltip :content="ImportData" effect="light">
          <el-icon class="icon-button" :size="16" @click="addRemote">
            <AddIcon />
          </el-icon>
        </el-tooltip>
        <el-tooltip :content="SwitchTheme" effect="light">
          <el-switch class="theme-toggle" v-model="isDarkTheme" @change="handleThemeChange"></el-switch>
        </el-tooltip>
        <ResourceDialog v-model:showModal="showModal" project-name=""></ResourceDialog>
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
    background: var(--mi-bg-color-light);
}

.theme-toggle {
    --el-switch-off-color: rgb(141, 141, 141);
    --el-switch-on-color: rgb(60, 60, 60);
}

.icon-button:hover {
    cursor: pointer;
}

.header .icon-container {
    margin-right: 0.5rem;
}
</style>