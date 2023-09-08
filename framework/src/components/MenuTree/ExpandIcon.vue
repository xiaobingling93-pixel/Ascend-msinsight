<script setup lang="ts">
import { defineProps } from 'vue';
import ArrowIcon from '../icons/arrow_icon.vue';
import IconContainer from '../IconContainer.vue';
import { indent } from './types';

defineProps<{ isExpanded: boolean }>();
</script>
<template>
    <IconContainer :width="indent">
        <ArrowIcon :transform="`rotate(${isExpanded ? 0 : -90})`"/>
    </IconContainer>
</template>
