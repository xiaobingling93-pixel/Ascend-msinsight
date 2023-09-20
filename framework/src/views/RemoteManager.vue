<script setup lang="ts">
import { ref, watch } from 'vue';
import AddIcon from '@/components/icons/cross_icon.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import ModalView from '@/components/ModalView.vue';
import FormComp from '@/components/FormComp.vue';
import { useDataSources } from '@/stores/dataSource';
import connector from '@/connection';
const isDarkTheme = ref(true);

watch(isDarkTheme, () => {
    connector.send({ body: { event: 'setTheme', body: { isDark: isDarkTheme.value } } });
});

const showModal = ref(false);
function addRemote(e: MouseEvent) {
    e.stopPropagation();
    showModal.value = true;
}

const store = useDataSources();
function confirm() {
    const isSuccess = store.confirm();
    showModal.value = !isSuccess;
}

function reset() {
    showModal.value = false;
    store.cancel();
}
</script>

<template>
    <header class="header">
        <el-icon class="icon-button" :size="20" @click="addRemote">
            <AddIcon/>
        </el-icon>
        <el-switch class="theme-toggle" v-model="isDarkTheme"></el-switch>
        <ModalView v-if="showModal" :close="reset" :confirm="confirm" title="Add your remote">
            <FormComp />
        </ModalView>
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
    padding: 0 15px;
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

.header .icon-container  {
    margin-right: 0.5rem;
}
</style>
