<!--
  - Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  -->

<script setup lang="ts">
import { useContextMenu } from '@/hooks/useContextMenu';
import { ref } from 'vue';
import { t } from '@/i18n';

interface MenuItem {
    label: string;
    key: string;
    action?: (...args: any[]) => void;
}
defineProps<{ menuItems: MenuItem[] }>();
const emit = defineEmits(['select']);

const containerRef = ref<HTMLElement>();
const { x, y, visible, setVisible } = useContextMenu(containerRef.value, '.can-right-click');

function clickMenuItem(item: MenuItem): void {
    setVisible(false);
    emit('select', item);
    if (item?.action && typeof item.action === 'function') {
        item.action();
    }
}
</script>

<template>
    <div class="container" ref="containerRef">
        <slot></slot>
        <Teleport to="body">
            <div class="context-menu" v-if="visible" :style="{ left: `${x}px`, top: `${y}px` }">
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
