<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { request } from '@/centralServer/server';
import connector from '@/connection';
import { modulesConfig } from '@/moduleConfig';
import { useSession, type Session } from '@/stores/session';
import { connectRemote } from '@/centralServer/server';
import { LOCAL_HOST, PORT } from '@/centralServer/websocket/defs';

const activeModule = ref(0);
const moduleRefs = ref<HTMLIFrameElement[] | undefined>();
const { session, setSession } = useSession();

onMounted(async () => {
    connector.resigsterAwaitFetch(async (e) => {
        const { remote, args, module } = e.data;
        const result = await request(remote, module, args);
        return { dataSource: remote, body: result };
    });

    connector.addListener('updateSession', (e) => {
        const receiver = e.data.body;
        if (!receiver) {
            console.warn('data.body is undefined, please check your params');
            return;
        }
        const receivePropKeys = Object.keys(receiver);
        const validPropKeys = Object.keys(session);
        const updateState = {};
        for (const key of receivePropKeys) {
            if (validPropKeys.includes(key) && Object.prototype.toString.call(receiver[key]) === Object.prototype.toString.call(session[key as keyof Session])) {
                Object.assign(updateState, { [key]: receiver[key] });
                continue;
            }
            console.warn(`you just send a invalid data: {${key}: ${receiver[key]}} to update session, please check it`);
        }
        setSession(updateState);
    });

    await connectRemote({ remote: LOCAL_HOST, port: PORT, dataPath: [] });
});

function toggleTab(index: number): void {
    activeModule.value = index;
    connector.send({
        body: {
            event: 'wakeup',
            body: {}
        },
        to: index,
    });
}

</script>
<template>
    <div class="tab-pane">
        <div class="tab-titles">
            <el-menu class="el-menu-title" mode="horizontal" background-color="#252526" router>
                <template
                    v-for="(moduleConfig, index) in modulesConfig"
                    :key="`${index}-${moduleConfig.name}`">
                <el-menu-item
                    v-if="moduleConfig.isDefault || session.isCluster"
                    @click="() => toggleTab(index)"
                    :class="index === activeModule && 'active'"
                >
                    {{ moduleConfig.name }}
                </el-menu-item>
                </template>
            </el-menu>
        </div>
        <div class="tab-body">
            <template v-for="(moduleConfig, index) in modulesConfig" 
                    :key="`${index}-${moduleConfig.name}`">
                <iframe
                    v-if="moduleConfig.isDefault || session.isCluster"
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

.el-menu {
  display: flex;
  border: none;
  height: 30px;
  width: 100%;
  line-height: 30px;
  border-bottom: none !important;
}

.el-menu-item {
    margin-right: 1px;
    text-align: center;
    width: 150px;
    font-size: 14px;
    color: #F4F6FA !important;
    line-height: 30px;
    user-select: none;
}

.el-menu-item.active {
  color: #007AFF !important;
}

.el-menu-item.active:after {
    content: '';
    position: absolute;
    bottom: 0;
    width: 64px;
    height: 1px;
    background-color: #007aff;
}

.el-menu-item:hover {
    background-color: #383838;
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
