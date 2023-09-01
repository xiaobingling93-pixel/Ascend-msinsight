<script setup lang="ts">
import { defineProps } from 'vue';
defineProps<{ click: (e: any) => void; type?: 'confirm' | 'cancel' }>();
</script>
<template>
    <button
        @click="click"
        :style="{ backgroundColor: type === 'cancel' ? '#ff4d4f' : '#0078d7' }"
        v-bind="$attrs"
    >
        <slot></slot>
    </button>
</template>

<style scoped>
button {
    color: #fff;
    border-radius: 5px;
    border: none;
    padding: 10px;
}
</style>
