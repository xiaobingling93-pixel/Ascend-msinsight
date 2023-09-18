<script setup lang="ts">
import { ref, watch } from 'vue';
import AddIcon from '@/components/icons/cross_icon.vue';
import IconContainer from '@/components/IconContainer.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import ModalView from '@/components/ModalView.vue';
import FormComp from '@/components/FormComp.vue';
import { useDataSources } from '@/stores/dataSource';
import connector from '@/connection';
const width = 1.3;
const boxShadow = '1px 1px 1px 1px var(--color-background-soft) inset, -1px -1px 1px 1px var(--color-background-soft) inset';
const isDarkTheme = ref(true);

watch(isDarkTheme, () => {
    connector.send({ event: 'setTheme', isDark: isDarkTheme.value });
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
        <IconContainer :width="width" :boxShadow="boxShadow" @click="addRemote">
            <AddIcon />
        </IconContainer>
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
    flex-direction: column;
    justify-content: start;
    align-items: center;
    min-width: 80px;
    background-color: #2C2C2C;
    border-right: var(--border-style);
}
.theme-toggle {
    --el-switch-off-color: rgb(141, 141, 141);
    --el-switch-on-color: rgb(60, 60, 60);
}
.container {
    flex-grow: 1;
    background-color: #252526;
}

.header .icon-container  {
    margin-right: 0.5rem;
}

@media (min-width: 1024px) {
    header {
        flex-direction: row;
        height: var(--header-height);
        border-right: unset;
    }
}
</style>
