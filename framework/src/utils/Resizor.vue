<script setup lang="ts">
import {ref} from 'vue';
import { Console } from '@/utils/console';

const emit = defineEmits(['onResize']);
const divRef: any = ref(null);
let isDown = false;
let offsetX: number;
let initalWidth: number;
let initalNextWidth: number;

function handleMouseDown(event: any): void {
  event.preventDefault();
  if (!isDown && Boolean(divRef.value)) {
    isDown = true;
    offsetX = event.clientX;
    const dom = divRef.value.parentNode;
    dom.style.pointerEvents = 'none';
    initalWidth = dom.offsetWidth;
    const nextBrother = dom.nextElementSibling;
    if (nextBrother !== null) {
      initalNextWidth = nextBrother.offsetWidth;
    }

    window.addEventListener('mousemove', handleMouseMove);
    window.addEventListener('mouseup', handleMouseUp);
    // 如果当前在iframe中
    if (self !== top) {
      window.addEventListener('message', handleTopWindow);
    }
    // 如果页面中有iframe
    if (window.document.querySelectorAll('iframe').length !== 0) {
      window.document.querySelectorAll('iframe').forEach((item: any) => {
        item.style['pointer-events'] = 'none';
      });
    }
  }
};

function handleMouseMove(event: any): void {
  event.preventDefault();
  if (isDown && divRef?.value as boolean) {
    const deltaX = event.clientX - offsetX;
    const dom = divRef.value.parentNode;
    const width = initalWidth + deltaX;
    const nextBrother = dom.nextElementSibling;
    if (nextBrother !== null) {
      const nextWidth = initalNextWidth - deltaX;
      emit('onResize', deltaX, width, nextWidth);
      return;
    }
    emit('onResize', deltaX, width);
  }
}
function handleMouseUp(event?: any): void {
  if (event !== undefined) {
    event.preventDefault();
  }
  isDown = false;
  offsetX = 0;
  window.removeEventListener('mousemove', handleMouseMove);
  window.removeEventListener('mouseup', handleMouseUp);
  const parentNode = divRef.value.parentNode;
  parentNode.style.pointerEvents = null;
  // 如果当前在iframe中
  if (self !== top) {
    window.removeEventListener('message', handleTopWindow);
  }
  // 如果页面中有iframe
  if (window.document.querySelectorAll('iframe').length !== 0) {
    window.document.querySelectorAll('iframe').forEach((item: any) => {
      item.style['pointer-events'] = 'auto';
    });
  }
}

function handleTopWindow(event?: any): void {
  try {
    if (typeof event.data !== 'string') {
      return;
    }
    const data = JSON.parse(event.data);
    if (data.from === 'framework' && data.event === 'mouseover') {
      handleMouseUp();
    }
  } catch (error) {
    Console.log(error);
  }
}

</script>

<template>
  <div class="resizor" @mousedown.native="handleMouseDown" ref="divRef"></div>
</template>

<style scoped>
.resizor {
  z-index:1000;
  position:absolute;
  height: 100%;
  width: 16px;
  top: 0;
  right:-8px;
  cursor: w-resize;
  pointer-events: all;
}
</style>