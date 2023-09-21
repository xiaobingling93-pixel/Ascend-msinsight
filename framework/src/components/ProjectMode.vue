<script setup lang="ts">
import ProjectIcon from '@/components/icons/project_icon.vue';
</script>

<template>
  <div class="projectMode">
    <el-icon class="projectModeIcon" :size="16">
      <ProjectIcon/>
    </el-icon>
    <span class="modeName">Remote Manager</span>
  </div>
</template>

<style>
.projectMode {
  display: flex;
  align-items: center;
  margin-right: auto;
  font-size: 12px;
  color: #989A9C;
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