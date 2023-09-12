<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { request } from '@/centralServer/server';
import connector from '@/connection';

const localPaths = [ './plugins/Timeline/index.html' ];

const getModuleName = (index: number): string => {
    return localPaths?.[index]?.replace('./plugins/', '').replace(/\/\w+.html/, '') ?? '';
};

const currentPath = ref(0);
const moduleRef = ref();

onMounted(() => {
    connector.resigsterAwaitFetch(async (e) => {
        const { remote, args } = e.data;
        const result = await request(remote, getModuleName(currentPath.value).toLowerCase(), args);
        return { dataSource: remote, body: result };
    });
});

function toggleTab(index: number): void {
    currentPath.value = index;
}

</script>
<template>
    <div class="tab-pane">
        <div class="tab-titles">
            <el-menu class="el-menu-title" mode="horizontal" background-color="#545c64" text-color="#fff" active-text-color="#ffd04b" router>
                <el-menu-item v-for="(path, index) in localPaths"
                              :key="`${index}-${path}`"
                              @click="() => toggleTab(index)"
                              :class="index === currentPath && 'active'"> {{ getModuleName(index) }}
                </el-menu-item>
            </el-menu>
        </div>
        <div class="tab-body">
            <iframe :src="localPaths[currentPath]" ref="moduleRef"></iframe>
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
