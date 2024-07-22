<script setup lang="ts">
defineProps<{ width: number; boxShadow?: string }>();
</script>

<template>
    <div class="icon-container" :style="{ width: `${width}rem`, boxShadow: `${boxShadow}` }">
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

.icon-container:hover {
    background-color: var(--mi-bg-color-dark);
}
</style>
