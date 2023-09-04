<script setup lang="ts">
import { ref } from 'vue';
import AddIcon from '@/components/icons/cross_icon.vue';
import GearIcon from '@/components/icons/gear_icon.vue';
import IconContainer from '@/components/IconContainer.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import ModalView from '@/components/ModalView.vue';
import FormComp from '@/components/FormComp.vue';
const width = 2;
let showModal = ref(false);
function addRemote(e: MouseEvent) {
    e.stopPropagation();
    showModal.value = true;
}
const mockData = [
    {
        content: '123',
        cancelable: true,
        children: [
            {
                content: '456',
                children: [],
            },
        ],
    },
];
</script>

<template>
    <header class="header">
        <IconContainer :width="width" @click="addRemote">
            <AddIcon />
        </IconContainer>
        <IconContainer :width="width">
            <GearIcon />
        </IconContainer>
        <ModalView v-if="showModal" :close="() => (showModal = false)" title="Add your remote">
            <FormComp />
        </ModalView>
    </header>
    <div class="container">
        <MenuTree :dataSource="mockData"></MenuTree>
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

@media (min-width: 1024px) {
    header {
        flex-direction: row;
        height: var(--header-height);
        border-right: unset;
        border-bottom: var(--border-style);
    }
}
</style>
