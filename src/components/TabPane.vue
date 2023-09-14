<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { request } from '@/centralServer/server';
import connector from '@/connection';
import { modulesConfig } from '@/moduleConfig';

const activeModule = ref(0);
const moduleRefs = ref<HTMLIFrameElement[] | undefined>();

onMounted(async () => {
    connector.resigsterAwaitFetch(async (e) => {
        const { remote, args, module } = e.data;
        const result = await request(remote, module, args);
        return { dataSource: remote, body: result };
    });
});

function toggleTab(index: number): void {
    activeModule.value = index;
}

</script>
<template>
    <div class="tab-pane">
        <div class="tab-titles">
            <el-menu class="el-menu-title" mode="horizontal" background-color="#545c64" text-color="#fff" active-text-color="#ffd04b" router>
                <el-menu-item v-for="(moduleConfig, index) in modulesConfig"
                    :key="`${index}-${moduleConfig.name}`"
                    @click="() => toggleTab(index)"
                    :class="index === activeModule && 'active'"> {{ moduleConfig.name }}
                </el-menu-item>
            </el-menu>
        </div>
        <div class="tab-body">
            <template v-for="(moduleConfig, index) in modulesConfig" 
                    :key="`${index}-${moduleConfig.name}`">
                <iframe
                    v-bind={...moduleConfig.attributes}
                    v-show="activeModule === index"
                    ref="moduleRefs"
                ></iframe>
            </template>
        </div>
    </div>
</template>

<style scoped>
.tab-pane {
    width: 100%;
    height: 100%;
    display: flex;
    flex-direction: column;
}

.tab-titles {
    height: var(--header-height);
}

.el-menu-title {
  display: flex;
  border: none;
  height: 30px;
  width: 100%;
  line-height: 30px;
  border-bottom: var(--border-style);
}

el-menu-item {
  background-color: #383838;
  margin-right: 1px;
  text-align: center;
  height: 28px !important;
  width: 100px;
  line-height: 30px;
  user-select: none;
}

el-menu-item.active {
  color: #FFD04B !important; /* 你想要的字体颜色 */
  border-bottom: 2px solid #FFD04B
}

el-menu-item:hover {
  background-color: #545C64;
}

@media (min-height: 1024px) {
    .tab-titles {
        height: 40px;
    }
}

.tab-body {
    flex-grow: 1;
}

@media (hover: hover) {
    a:hover {
        background-color: var(--color-background-medium-active);
    }
}

iframe {
    width: 100%;
    height: 100%;
    border: 0;
}
</style>
