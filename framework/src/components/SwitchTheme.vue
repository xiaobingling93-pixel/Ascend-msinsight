<script setup lang="ts">
import useWatchTranslation from '@/hooks/useWatchTranslation';
import { onMounted, ref } from 'vue';
import { LocalStorageKeys, localStorageService } from '@/utils/local-storage';
import connector from '@/connection';
import {useSession} from '@/stores/session';
import ThemeLightIcon from '@/components/icons/theme_light.vue';
import ThemeDarkIcon from '@/components/icons/theme_dark.vue';

const { session, setSession } = useSession();

const [SwitchTheme] = useWatchTranslation(['Switch Theme']);
const lsTheme = localStorageService.getItem(LocalStorageKeys.THEME) ?? 'dark';
const isDarkTheme = ref(lsTheme === 'dark');

onMounted(() => {
    changeElementTheme(isDarkTheme.value);
    connector.addListener('getParseStatus', () => {
        sendThemeEvent(isDarkTheme.value);
    });
    connector.addListener('getTheme', () => {
        sendThemeEvent(isDarkTheme.value);
    });
    setSession({theme: lsTheme});
});

function sendThemeEvent(isDark: boolean) {
    connector.send({
      event: 'setTheme',
      body: { isDark },
      target: 'plugin',
    });
}

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
        target: 'plugin',
    });
    document.body.className = isDark ? 'dark-theme' : 'light-theme';
    setSession({theme});
}
</script>

<template>
    <el-tooltip :content="SwitchTheme" effect="light">
        <el-switch class="switch-theme" v-model="isDarkTheme" @change="handleThemeChange">
            <template #active-action>
                <ThemeDarkIcon />
            </template>
            <template #inactive-action>
                <ThemeLightIcon />
            </template>
        </el-switch>
    </el-tooltip>
</template>

<style scoped>
.switch-theme {
    --el-switch-off-color: var(--mi-border-color-lighter);
    --el-switch-on-color: var(--mi-border-color-lighter);
}
</style>
