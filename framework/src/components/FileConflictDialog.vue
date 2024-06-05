<script setup lang="ts">
import useWatchTranslation from '@/hooks/useWatchTranslation';

const props = defineProps<{ dialogCoverVisible: boolean; projectName: string }>();
const [Confirm, Cancel, FileConflict, FileConflictContent] = useWatchTranslation(['Confirm', 'Cancel', 'FileConflict', 'FileConflictContent']);
const emit=defineEmits(['cover-file']);

</script>

<template>
  <el-dialog :model-value="dialogCoverVisible" :title="FileConflict" width="20%" :show-close="false" :align-center="true" @close="emit('cover-file', false)">
    <span>{{ FileConflictContent }}</span>
    <template #footer>
      <span class="dialog-footer">
          <el-button @click="emit('cover-file', false)">{{ Cancel }}</el-button>
          <el-button type="primary" @click="emit('cover-file', true)"> {{ Confirm }} </el-button>
      </span>
    </template>
  </el-dialog>
</template>

<style scoped>

</style>