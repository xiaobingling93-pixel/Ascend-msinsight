<script setup lang="ts">
import { onMounted, ref } from 'vue';
import { ArrowLeft, ArrowRight } from '@element-plus/icons-vue';
import RemoteManager from './views/RemoteManager.vue';
import Modules from './views/ModulesView.vue';
const displayAside = ref(true);
const asideWidth = ref(300);
const handleDisplayAside = () => {
    displayAside.value = !displayAside.value;
    asideWidth.value = displayAside.value ? 300 : 0;
}
const theme = ref('dark-theme');

onMounted(() => {
    document.body.className = theme.value;
})
</script>

<template>
    <el-container class="container">
        <el-aside class="aside" :width="`${asideWidth}px`">
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
