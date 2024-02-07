<script setup lang="ts">
import { onMounted, ref } from 'vue';
import { ArrowLeft, ArrowRight } from '@element-plus/icons-vue';
import RemoteManager from './views/RemoteManager.vue';
import Modules from './views/ModulesView.vue';
import Resizor from '@/utils/Resizor.vue';
import { useSession, type Session } from '@/stores/session';
let lastWidth = 300;
const displayAside = ref(true);
const asideWidth = ref(lastWidth);
const handleDisplayAside = () => {
    displayAside.value = !displayAside.value;
    asideWidth.value = displayAside.value ? lastWidth : 0;
};
const theme = ref('dark-theme');
const { session } = useSession();

const forbidDefaultEvent = (e: MouseEvent) => {
    e.preventDefault();
};

onMounted(() => {
    // 默认颜色主题
    document.body.className = theme.value;
    // 禁用文件拖拽
    window.addEventListener('drop', forbidDefaultEvent);
    window.addEventListener('dragover', forbidDefaultEvent);
    // 鼠标在framework滑动
    window.addEventListener('mouseover', () => {
        for (let i = 0; i < window.frames.length; i++) {
            window.frames[i].postMessage(JSON.stringify({ from: 'framework', event: 'mouseover' }), window.origin);
        }
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
                <el-icon>
                    <ArrowLeft v-if="displayAside"/>
                    <ArrowRight v-else/>
                </el-icon>
            </div>
        </el-aside>
        <el-main class="main">
            <Modules />
        </el-main>
    </el-container>
</template>

<style scoped>
.container {
    width: 100vw;
    height: 100vh;
}

.el-aside {
    background-color: var(--color-background);
}

.container >>> .el-loading-mask {
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
}

.aside-handler:hover {
    background-color: rgb(90, 90, 90);
}

main {
    padding: 0;
}
</style>
