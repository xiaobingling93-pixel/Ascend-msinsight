<script setup lang="ts">
import {nextTick, ref} from 'vue';
import {type TreeNodeType} from '@/components/MenuTree/types';
import {useDataSources} from '@/stores/dataSource';

const props = defineProps<{
  treeNode: TreeNodeType;
}>();

const editing = ref(false);
const editText = ref(props.treeNode.label);
const inputRef = ref();

const showEditBox = () => {
  editing.value = true;
};

const hideEditBox = async () => {
  if (editText.value === '') {
    editing.value = false;
    editText.value = props.treeNode.label;
    return;
  }
  if (props.treeNode.label !== editText.value) {
    // 更新dataSource
    const res = await useDataSources().updateProjectName(props.treeNode.label, editText.value);
    if (!res) {
      editing.value = false;
      editText.value = props.treeNode.label;
    }
  }
  editing.value = false;
};

const handleContextMenu = (event: { preventDefault: () => void; }) => {
  showEditBox();
  nextTick(() => {
    inputRef.value.focus();
  });
};

const blurInput = () => {
  inputRef.value.blur();
};

</script>

<template>
  <div class="title-box">
    <div v-show="!editing" @contextmenu="handleContextMenu" class="content-text">{{ editText }}</div>
    <input v-show="editing" ref="inputRef" type="text" v-model="editText" @click.stop @blur="hideEditBox" @keyup.enter="blurInput" maxlength="500" autofocus>
  </div>
</template>

<style scoped>
.title-box {
    overflow: hidden;
}
.content-text {
    color: var(--mi-text-color-primary);
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
}
</style>