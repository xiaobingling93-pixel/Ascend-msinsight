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
            <nav>
                <a
                    v-for="(path, index) in localPaths"
                    :key="`${index}-${path}`"
                    @click="() => toggleTab(index)"
                    :class="index === currentPath && 'active'"
                >
                    {{ getModuleName(index) }}
                </a>
            </nav>
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

@media (min-height: 1024px) {
    .tab-titles {
        height: 40px;
    }
}

.tab-body {
    flex-grow: 1;
}

nav {
    width: 100%;
    height: 100%;
    font-size: 1rem;
    text-align: left;
    border-bottom: var(--border-style);
    display: flex;
    align-items: flex-end;
}

nav a.active {
    color: var(--color-text);
    background-color: var(--color-background-medium);
}

nav a:first-of-type {
    margin-left: 1px;
}

nav a {
    display: inline-block;
    border-radius: 0.5rem 0.5rem 0 0;
    padding: 0 1rem;
    border-left: var(--border-style);
    border-right: var(--border-style);
    border-top: var(--border-style);
    height: 90%;
    min-width: 10rem;
    text-align: center;
    margin-right: 1px;
    text-decoration: none;
    color: var(--color-text);
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
