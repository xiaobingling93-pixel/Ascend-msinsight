<script setup lang="ts">
import {computed, onMounted, ref} from 'vue';
import { ElConfigProvider, ElNotification } from 'element-plus';
import zhCN from 'element-plus/es/locale/lang/zh-cn';
import enUS from 'element-plus/es/locale/lang/en';
import ArrowLeftDarkIcon from '@/components/icons/arrow_left_dark_icon.vue';
import ArrowLeftLightIcon from '@/components/icons/arrow_left_light_icon.vue';
import ArrowRightDarkIcon from '@/components/icons/arrow_right_dark_icon.vue';
import ArrowRightLightIcon from '@/components/icons/arrow_right_light_icon.vue';
import RemoteManager from './views/RemoteManager.vue';
import Modules from './views/ModulesView.vue';
import Resizor from '@/utils/Resizor.vue';
import { useSession } from '@/stores/session';
import connection from '@/connection';
import {LOCAL_HOST, PORT} from '@/centralServer/websocket/defs';
import {LocalStorageKeys, localStorageService} from '@/utils/local-storage';
import { useDataSources } from '@/stores/dataSource';
import { useLoading } from "@/hooks/useLoading";
import useWatchTranslation from "@/hooks/useWatchTranslation";

let lastWidth = 300;
const displayAside = ref(true);
const asideWidth = ref(lastWidth);
const handleDisplayAside = () => {
    displayAside.value = !displayAside.value;
    asideWidth.value = displayAside.value ? lastWidth : 0;
};
const { session } = useSession();

const locales = {
  zhCN,
  enUS
};

type Language = 'zhCN' | 'enUS';

const locale = computed(() => locales[session.language as Language] ?? enUS);

const {checkConflict, confirm} = useDataSources();

const [FileConflict, FileConflictContent] = useWatchTranslation(['FileConflict', 'FileConflictContent'])

onMounted(() => {
    // 获取主题记忆信息，并设置主题
    const curTheme = localStorageService.getItem(LocalStorageKeys.THEME) === 'dark';
    document.body.className = curTheme ? 'dark-theme' : 'light-theme';
    // 鼠标在framework滑动
    window.addEventListener('mouseover', () => {
        for (let i = 0; i < window.frames.length; i++) {
            window.frames[i].postMessage(JSON.stringify({ from: 'framework', event: 'mouseover' }), window.origin);
        }
    });

    Object.defineProperty(window, 'handleDrop', {
      value: async (path: string) => {
        const dataSource = {
          remote: LOCAL_HOST,
          port: PORT,
          projectName: path,
          dataPath: [path]
        };

        const isConflict = await checkConflict(dataSource);

        if (isConflict) {
          ElNotification.info({
            title: <string>FileConflict.value,
            message: <string>FileConflictContent.value,
            showClose: false,
          });
        } else {
          await confirm(dataSource, false);
        }
      },
      writable: true,
      enumerable: false,
      configurable: true
    });
});

function resize(deltaX: number, width: number) {
    if (width >= 200 && width <= 1000) {
        asideWidth.value = width;
        lastWidth = width;
    }
}
</script>

<template>
  <el-config-provider :locale="locale">
    <el-container v-loading="session.loading" class="container">
        <el-aside class="aside" :width="`${asideWidth}px`">
          <div :style="`width:${asideWidth}px;height:100%;position:absolute;`">
            <Resizor @onResize="resize"/>
          </div>
            <RemoteManager />
            <div
                class="aside-handler"
                :style="{ left: `${Math.max(0, asideWidth - 20)}px` }"
                @click="handleDisplayAside"
            >
                <template v-if="displayAside">
                    <ArrowLeftDarkIcon v-if="session.theme=='dark'"/>
                    <ArrowLeftLightIcon v-else/>
                </template>
                <template v-else>
                    <ArrowRightDarkIcon v-if="session.theme=='dark'"/>
                    <ArrowRightLightIcon v-else/>
                </template>
            </div>
        </el-aside>
        <el-main class="main">
            <Modules />
        </el-main>
    </el-container>
  </el-config-provider>
</template>

<style scoped>
.container {
    width: 100vw;
    height: 100vh;
}

.el-aside {
    background-color: var(--mi-bg-color);
    border-right: 1px solid var(--mi-bg-color-dark);
}

.container :deep(.el-loading-mask) {
    opacity: 0.7;
}

.aside-handler {
    position: absolute;
    display: flex;
    align-items: center;
    justify-content: center;
    top: 50%;
    height: 50px;
    width: 20px;
    cursor: pointer;
}

main {
    padding: 0;
}
</style>
