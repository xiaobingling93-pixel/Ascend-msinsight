<script setup lang="ts">
import {nextTick, ref} from 'vue';
import {type TreeNodeType} from '@/components/MenuTree/types';
import {useDataSources} from '@/stores/dataSource';
import {HandleSingleDoubleClick} from '@/utils';

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

const enterEditStatus = () => {
  showEditBox();
  nextTick(() => {
    inputRef.value.focus();
  });
};

const handleDoubleClick = () => {
  // Vue不区分单击、双击,为避免单击事件运行，增加额外控制
  HandleSingleDoubleClick.doubleClick(() => {
    enterEditStatus();
  }, 'projectName');
};

const blurInput = () => {
  inputRef.value.blur();
};

</script>

<template>
  <div class="title-box">
    <div v-show="!editing" @dblclick="handleDoubleClick" class="content-text can-right-click">{{ editText }}</div>
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

.is-baseline > .el-tree-node__content .content-text {
  color: var(--mi-color-primary);
  font-weight: bold;
}
</style>