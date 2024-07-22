<script setup lang="ts">
import BinIcon from '../icons/bin_icon.vue';
import IconContainer from '../IconContainer.vue';
import { indent } from './types';
</script>
<template>
    <IconContainer :width="indent">
        <BinIcon class="mi-icon" />
    </IconContainer>
</template>
