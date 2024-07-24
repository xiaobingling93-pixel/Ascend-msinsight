<script setup lang="ts">
import ProjectDarkIcon from '@/components/icons/project_dark_icon.vue';
import ProjectLightIcon from '@/components/icons/project_light_icon.vue';
import {useWatchTranslation} from '@/hooks/useWatchTranslation';
import {useSession} from '@/stores/session';

const { session } = useSession();
const [DataManager] =  useWatchTranslation(['Data Manager']);
</script>

<template>
  <div class="projectMode">
    <el-icon class="projectModeIcon" :size="16">
        <ProjectDarkIcon v-if="session.theme=='dark'"/>
        <ProjectLightIcon v-else/>
    </el-icon>
    <span class="modeName">{{ DataManager }}</span>
  </div>
</template>

<style>
.projectMode {
  display: flex;
  align-items: center;
  margin-right: auto;
  font-size: 12px;
  line-height: 18px;
  font-weight: 400;
}
.projectModeIcon {
  margin-right: 4px;
}
.modeName {
  margin-right: 2px;
  user-select: none;
}
.dropDownIcon {
  margin-top: 3px;
}
</style>