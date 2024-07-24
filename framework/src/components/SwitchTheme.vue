<script setup lang="ts">
import useWatchTranslation from '@/hooks/useWatchTranslation';
import { onMounted, ref } from 'vue';
import { LocalStorageKeys, localStorageService } from '@/utils/local-storage';
import connector from '@/connection';
import {useSession} from '@/stores/session';

const { session, setSession } = useSession();

const [SwitchTheme] = useWatchTranslation(['Switch Theme']);
const isDarkTheme = ref(localStorageService.getItem(LocalStorageKeys.THEME) === 'dark');

onMounted(() => {
    changeElementTheme(isDarkTheme.value);
    connector.addListener('getParseStatus', () => {
        connector.send({
            event: 'setTheme',
            body: { isDark: isDarkTheme.value },
        });
    });
    setSession({theme: localStorageService.getItem(LocalStorageKeys.THEME)});
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
    setSession({theme});
}
</script>

<template>
    <el-tooltip :content="SwitchTheme" effect="light">
        <el-switch class="switch-theme" v-model="isDarkTheme" @change="handleThemeChange"></el-switch>
    </el-tooltip>
</template>

<style scoped>
.switch-theme {
    --el-switch-off-color: var(--mi-bg-color-dark);
    --el-switch-on-color: var(--mi-bg-color-dark);
}
</style>
