<!--
  - Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  -->

<script setup lang="ts">
import { useContextMenu } from '@/hooks/useContextMenu';
import { computed, ref } from 'vue';
import { t } from '@/i18n';
import { useViewport } from '@/hooks/useViewport';
import type {MenuItem} from '@/components/MenuTree/types';

defineProps<{ menuItems: MenuItem[] }>();
const emit = defineEmits(['select']);

const containerRef = ref<HTMLElement>();
const { x, y, visible, setVisible } = useContextMenu(containerRef.value, '.can-right-click');
const { vw, vh } = useViewport();
const menuWidth = ref(0);
const menuHeight = ref(0);

const pos = computed(() => {
    let posX = x.value;
    let posY = y.value;
    if (posX > vw.value - menuWidth.value) {
        posX -= menuWidth.value;
    }
    if (posY > vh.value - menuHeight.value) {
        posY = vh.value - menuHeight.value;
    }
    return { posX, posY };
});

function clickMenuItem(item: MenuItem): void {
    setVisible(false);
    emit('select', item);
    if (item?.action && typeof item.action === 'function') {
        item.action();
    }
}

function handleSizeChange({ width, height }: { width: number; height: number }) {
    menuWidth.value = width;
    menuHeight.value = height;
}
</script>

<template>
    <div class="container" ref="containerRef">
        <slot></slot>
        <Teleport to="body">
            <div class="context-menu" v-if="visible&&menuItems.length>0" v-size-ob="handleSizeChange" :style="{ left: `${pos.posX}px`, top: `${pos.posY}px` }">
                <div class="context-menu-item" v-for="item in menuItems" :key="item.label" @mousedown.stop @click="clickMenuItem(item)">
                    {{ t(item.label) }}
                </div>
            </div>
        </Teleport>
    </div>
</template>

<style scoped>
.container {
    position: relative;
    height: calc(100vh - 84px);
    overflow: auto;
    margin-right: 10px;
}

.context-menu {
    z-index: 99999;
    position: fixed;
    min-width: 150px;
    padding: 4px 0;
    overflow: hidden;
    font-size: 12px;
    color: var(--mi-text-color-primary);
    border-radius: var(--mi-border-radius-base);
    background-color: var(--menu-bg-color);
    box-shadow: var(--mi-box-shadow);
    transition: all 0.1s ease;
}

.context-menu-item {
    padding: 8px 12px;
    user-select: none;
}

.context-menu-item:hover {
    background-color: var(--mi-color-primary);
    color: #ffffff;
}
</style>
