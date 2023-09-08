<script setup lang="ts">
import { ref } from 'vue';
import AddIcon from '@/components/icons/cross_icon.vue';
import GearIcon from '@/components/icons/gear_icon.vue';
import IconContainer from '@/components/IconContainer.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import ModalView from '@/components/ModalView.vue';
import FormComp from '@/components/FormComp.vue';
import { useDataSources } from '@/stores';
const width = 1.3;
const boxShadow = '1px 1px 1px 1px var(--color-background-soft) inset, -1px -1px 1px 1px var(--color-background-soft) inset';
let showModal = ref(false);
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
        <IconContainer :width="width" :boxShadow="boxShadow">
            <GearIcon />
        </IconContainer>
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
    border-right: var(--border-style);
}

.container {
    flex-grow: 1;
}

.header .icon-container  {
    margin-right: 0.5rem;
}

@media (min-width: 1024px) {
    header {
        flex-direction: row;
        height: var(--header-height);
        border-right: unset;
        border-bottom: var(--border-style);
    }
}
</style>
