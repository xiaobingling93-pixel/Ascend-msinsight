<script setup lang="ts">
import useWatchTranslation from '@/hooks/useWatchTranslation';
import { ProjectErrorType } from '@/utils/enmus';
import { reactive, computed } from 'vue';

const props = defineProps<{ dialogCoverVisible: boolean; projectName: string; projectCheckResult: ProjectErrorType }>();
const [Confirm, Cancel, FileConflict, FileConflictContent, FileUnsafe, FileUnsafeContent, LargeFile, LargeFileContent ] =
    useWatchTranslation(['Confirm', 'Cancel', 'FileConflict', 'FileConflictContent', 'FileUnsafe', 'FileUnsafeContent',
                         'LargeFile', 'LargeFileContent']);

const emit=defineEmits(['cover-file']);

const CONTENT_MAP = new Map([
  [ProjectErrorType.PROJECT_NAME_CONFLICT, FileConflictContent],
  [ProjectErrorType.IS_UNSAFE_PATH, FileUnsafeContent],
  [ProjectErrorType.EXISTING_LARGE_FILES, LargeFileContent]
]);

const TTITLE_MAP = new Map([
  [ProjectErrorType.PROJECT_NAME_CONFLICT, FileConflict],
  [ProjectErrorType.IS_UNSAFE_PATH, FileUnsafe],
  [ProjectErrorType.EXISTING_LARGE_FILES, LargeFile]
]);

// 一个计算属性 ref
const alert_title = computed(() => {
  if (props.projectCheckResult && TTITLE_MAP.has(props.projectCheckResult)) {
    return TTITLE_MAP.get(props.projectCheckResult)?.value;
  }
  return '';
});

// 一个计算属性 ref
const alert_content = computed(() => {
  if (props.projectCheckResult && CONTENT_MAP.has(props.projectCheckResult)) {
    return CONTENT_MAP.get(props.projectCheckResult)?.value;
  }
  return '';
});

</script>

<template>
  <el-dialog :model-value="dialogCoverVisible" :title="alert_title" width="20%" :show-close="false" :align-center="true" @close="emit('cover-file', false)">
    <span> {{ alert_content }}</span>
    <template #footer>
      <span class="dialog-footer">
          <el-button @click="emit('cover-file', false)">{{ Cancel }}</el-button>
          <el-button v-if="projectCheckResult !== ProjectErrorType.IS_UNSAFE_PATH" type="primary" @click="emit('cover-file', true)"> {{ Confirm }} </el-button>
      </span>
    </template>
  </el-dialog>
</template>

<style scoped>

</style>