<script setup lang="ts">
import { computed, onMounted, reactive, ref, watch } from 'vue';
import AddIcon from '@/components/icons/cross_icon.vue';
import MenuTree from '@/components/MenuTree/MenuTree.vue';
import ResourceComp from '@/components/ResourceComp.vue';
import { useDataSources } from '@/stores/dataSource';
import connector from '@/connection';
import ProjectMode from '@/components/ProjectMode.vue';
import { ElMessage } from 'element-plus';
import { isWindows } from '@/utils/is';

const MAX_FILE_PATH_LENGTH_WINDOWS = 260;
const MAX_FILE_PATH_LENGTH_LINUX = 4096;

const isDarkTheme = ref(true);
const resourceComp = ref();
// 输入文件是否存在
const fileIsExist = ref(false);
// 按钮是否可点击
const clickAble = ref(true);
const maxPathLen = isWindows ? MAX_FILE_PATH_LENGTH_WINDOWS : MAX_FILE_PATH_LENGTH_LINUX;
const inputLen = ref(0);

const isDisabled = computed(() => !fileIsExist.value || !clickAble.value || inputLen.value > maxPathLen);
onMounted(() => {
    connector.addListener('getParseStatus', () => {
      connector.send({
        event: 'setTheme',
        body: { isDark: isDarkTheme.value },
      });
    });
});
watch(isDarkTheme, () => {
    connector.send({
        event: 'setTheme',
        body: { isDark: isDarkTheme.value },
    });
    document.body.className = document.body.className === 'dark-theme' ? 'light-theme' : 'dark-theme';
});
const changeConfirmButtonState = (buttonState: boolean) => {
  fileIsExist.value = buttonState;
};
const showModal = ref(false);
function addRemote(e: MouseEvent) {
    e.stopPropagation();
    showModal.value = true;
    if (resourceComp.value) {
      resourceComp.value.doWhileOpenDialog();
    }
}

const store = useDataSources();

let clicking=false;
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

const handleConfirm = () => {
    showModal.value = false;
    setTimeout(() => {
        const result = resourceComp.value.doSetCurrentPath();
        if (!result) {
            ElMessage.error('Error');
        }
    }, 100);
};

function onInputChange(val:number) {
  inputLen.value = val;
}
</script>

<template>
    <header class="header">
        <ProjectMode />
        <el-tooltip content="Import Data" :effect="isDarkTheme ? 'light' : 'dark'">
          <el-icon class="icon-button" :size="16" @click="addRemote">
            <AddIcon />
          </el-icon>
        </el-tooltip>
        <el-tooltip content="Switch Theme" :effect="isDarkTheme ? 'light' : 'dark'">
          <el-switch class="theme-toggle" v-model="isDarkTheme"></el-switch>
        </el-tooltip>
        <el-dialog v-model="showModal" title="File Explorer" width="30%" :close-on-click-modal="false">
            <ResourceComp ref="resourceComp" :max-path-len="maxPathLen" @input-change="onInputChange" :changeConfirmButtonState = "changeConfirmButtonState" />
            <template #footer>
                <div class="foot-tip">
                    To refresh a directory, collapse and expand the directory.
                </div>
                <span>
                    <el-button :disabled="isDisabled" type="primary" @click="addClickProtect(handleConfirm)">Confirm</el-button>
                    <el-button @click="showModal = false">Cancel</el-button>
                </span>
            </template>
        </el-dialog>
    </header>
    <div class="container">
        <MenuTree :dataSource="store.menuTree" :is-dark-theme="isDarkTheme"></MenuTree>
    </div>
</template>

<style scoped>
header {
    display: flex;
    justify-content: end;
    gap: 7px;
    flex-direction: row;
    height: var(--header-height);
    align-items: center;
    min-width: 80px;
    padding: 0 15px 0 5px;
    background-color: var(--header-background-color);
}

.theme-toggle {
    --el-switch-off-color: rgb(141, 141, 141);
    --el-switch-on-color: rgb(60, 60, 60);
}

.container {
    flex-grow: 1;
    background-color: var(--color-background);
}

.icon-button {
    border-radius: var(--border-radius);
}

.icon-button:hover {
    cursor: pointer;
}

.header .icon-container {
    margin-right: 0.5rem;
}

.foot-tip {
  font-size: 14px;
  padding-bottom: 10px;
  text-align: left;
  color: var(--el-color-warning);
}
</style>