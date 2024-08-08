<script setup lang="ts">
import ResourceComp from '@/components/ResourceComp.vue';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import {computed, onMounted, ref} from 'vue';
import {isWindows} from '@/utils/is';
import { ProjectErrorType } from '@/utils/enmus';
import {ElMessage} from 'element-plus';
import FileConflictDialog from '@/components/FileConflictDialog.vue';
import {useLoading} from '@/hooks/useLoading';

const props = defineProps<{ showModal: boolean; projectName: string }>();
const emit=defineEmits(['update:showModal']);

const inputLen = ref(0);
const resourceComp = ref();
// 输入文件是否存在
const fileIsExist = ref(false);
// 按钮是否可点击
const clickAble = ref(true);
const dialogCoverVisible = ref(false);

const MAX_FILE_PATH_LENGTH_WINDOWS = 260;
const MAX_FILE_PATH_LENGTH_LINUX = 4096;
const maxPathLen = isWindows ? MAX_FILE_PATH_LENGTH_WINDOWS : MAX_FILE_PATH_LENGTH_LINUX;
const isDisabled = computed(() => !fileIsExist.value || !clickAble.value || inputLen.value > maxPathLen);

const [Confirm, Cancel, FileExplorer, RefreshDirectoryDescribe, CurProject] = useWatchTranslation(['Confirm', 'Cancel', 'File Explorer', 'RefreshDirectoryDescribe', 'Current Project']);

onMounted(() => {
  if (resourceComp.value) {
      resourceComp.value.doWhileOpenDialog();
    }
});

const projectCheckResult = ref(ProjectErrorType.NO_ERRORS);

const changeConfirmButtonState = (buttonState: boolean) => {
  fileIsExist.value = buttonState;
};

function onInputChange(val:number) {
  inputLen.value = val;
}

const addClickProtect = (func: () => void): void => {
  if (!clickAble.value) {
    return;
  }
  clickAble.value = false;
  func();
  setTimeout(() => {
    clickAble.value = true;
  }, 1000);
};

const handleConfirm = async () => {
    useLoading().open({});
    const result = await resourceComp.value.doCheckFileVallid(props.projectName);
    if (result != ProjectErrorType.NO_ERRORS) {
      useLoading().close();
      dialogCoverVisible.value = true;
      projectCheckResult.value = result;
    } else {
      const setPathResult = resourceComp.value.doSetCurrentPath(props.projectName, false);
      emit('update:showModal', false);
      if (!setPathResult) {
        ElMessage.error('Error');
      }
    }
};

const handleCoverVisible = (value: boolean) => {
  if (value) {
    const setPathResult = resourceComp.value.doSetCurrentPath(props.projectName, true);
    if (!setPathResult) {
      ElMessage.error('Error');
    }
    emit('update:showModal', false);
  }
  dialogCoverVisible.value = false;
};

</script>

<template>
  <el-dialog :model-value="showModal" :title="FileExplorer" width="800px" :close-on-click-modal="false" @close="emit('update:showModal', false)">
    <el-tooltip v-if="projectName" :content="projectName">
      <span class="project-name-span">{{ CurProject }} ：{{ projectName }}</span>
    </el-tooltip>
    <FileConflictDialog :dialog-cover-visible="dialogCoverVisible" :project-name="projectName" :project-check-result="projectCheckResult" @cover-file="handleCoverVisible"></FileConflictDialog>
    <ResourceComp ref="resourceComp" :show="showModal" :max-path-len="maxPathLen" @input-change="onInputChange" :changeConfirmButtonState = "changeConfirmButtonState" />
    <template #footer>
      <span>
          <el-button class="btn" :disabled="isDisabled" type="primary" @click="addClickProtect(handleConfirm)">{{ Confirm }}</el-button>
          <el-button class="btn" @click="emit('update:showModal', false)">{{ Cancel }}</el-button>
      </span>
    </template>
  </el-dialog>
</template>

<style scoped>
.btn {
  min-width: 56px;
}
.project-name-span {
  display: flow;
  white-space: nowrap;
  text-overflow: ellipsis;
  overflow: hidden;
  font-weight: 700;
  padding-bottom: 10px;
}
</style>