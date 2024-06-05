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

const hideEditBox = () => {
  if (editText.value === '') {
    editing.value = false;
    editText.value = props.treeNode.label;
    return;
  }
  if (props.treeNode.label !== editText.value) {
    // 更新dataSource
    useDataSources().updateProjectName(props.treeNode.label, editText.value);
  }
  editing.value = false;
};

const handleContextMenu = (event: { preventDefault: () => void; }) => {
  showEditBox();
  nextTick(() => {
    inputRef.value.focus();
  });
};

</script>

<template>
  <div>
    <span v-show="!editing" @contextmenu="handleContextMenu" class="contentText">{{ editText }}</span>
    <input v-show="editing" ref="inputRef" type="text" v-model="editText" @click.stop @blur="hideEditBox" @keyup.enter="hideEditBox" maxlength="500" autofocus>
  </div>
</template>

<style scoped>

</style>