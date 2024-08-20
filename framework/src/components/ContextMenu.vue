<script setup lang="ts">
import { ref } from 'vue';
const props = defineProps<{ menuVisible: boolean, menuItems: { label: string; action: () => void }[], position: { top: string, left: string} }>();
const emit = defineEmits(['update:menuVisible']);

</script>

<template>
  <div v-if="menuVisible" class="context-menu" :style="position" @click="emit('update:menuVisible', false)">
    <ul>
      <li v-for="(item, index) in menuItems" :key="index" @click="() => item.action && item.action()">
        {{ item.label }}
      </li>
    </ul>
  </div>
</template>
<style scoped>
.context-menu {
  min-width: 200px;
  border: 1px solid #ccc;
  position: fixed;
  z-index: 99999;
  background-color: blue;
  transition: all .1s ease;
}

.context-menu ul {
  list-style: none;
  padding: 0;
  margin: 0;
}

.context-menu ul li {
  padding: 8px 12px;
  cursor: pointer;
}

.context-menu ul li:hover {
  background-color: #a2a2a9;
}
</style>