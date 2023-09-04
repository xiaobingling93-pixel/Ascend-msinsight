<script setup lang="ts">
import { defineProps } from 'vue';
import ButtonComp from '@/components/ButtonComp.vue';
defineProps<{ title: string; close: () => void }>();
</script>
<template>
    <div class="modal-view-container">
        <div class="modal-view-header">{{ title }}</div>
        <div class="modal-view-content">
            <slot></slot>
        </div>
        <div class="modal-view-footer">
            <ButtonComp :click="close" :type="'cancel'">cancel</ButtonComp>
            <ButtonComp :click="close">confirm</ButtonComp>
        </div>
    </div>
</template>
<style>
@keyframes jump {
    from {
        transform: scale(0.01) translate(0, 0);
    }

    to {
        transform: scale(1) translate(35vw, 32.5vh);
    }
}

.modal-view-container {
    position: fixed;
    left: 0;
    top: 0;
    min-width: 350px;
    min-height: 250px;
    width: 30vw;
    height: 25vh;
    animation: jump 0.1s linear forwards;
    background-color: var(--color-background);
    --border-radius: 1.5rem;
    border-radius: var(--border-radius);
    --padding: 0.8rem 2rem;
    display: flex;
    flex-direction: column;
}

.modal-view-header {
    padding: var(--padding);
    line-height: 1rem;
}

.modal-view-footer {
    padding: var(--padding);
    position: relative;
    left: calc(100% - 200px);
}

.modal-view-footer > button:first-of-type {
    margin-right: 1rem;
}

.modal-view-content {
    padding: var(--padding);
    overflow-y: auto;
    overflow-x: hidden;
    flex-grow: 1;
}
</style>
