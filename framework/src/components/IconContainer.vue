<script setup lang="ts">
import { defineProps } from 'vue';
defineProps<{ width: number }>();
</script>

<template>
    <div class="icon-container" :style="`width: ${width}rem`">
        <slot></slot>
    </div>
</template>
<style scoped>
.icon-container {
    aspect-ratio: 1;
    overflow: hidden;
    padding: 0.1rem;
    border-radius: 50%;
    display: flex;
    justify-content: center;
    align-items: center;
}
</style>
