<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { request } from '@/centralServer/server';
import connector from '@/connection';
import { modulesConfig } from '@/moduleConfig';
import { useSession, type Session } from '@/stores/session';
import { connectRemote } from '@/centralServer/server';
import { LOCAL_HOST, PORT, setPort } from '@/centralServer/websocket/defs';
import { useDataSources } from '@/stores/dataSource';

const activeModule = ref(0);
const moduleRefs = ref<HTMLIFrameElement[] | undefined>();
const { session, setSession } = useSession();

onMounted(async () => {
    connector.resigsterAwaitFetch(async (e) => {
        if (!e.data.remote) {
          e.data.remote = useDataSources().lastDataSource;
        }
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
        const updateState: any = {};
        for (const key of receivePropKeys) {
            if (validPropKeys.includes(key) && Object.prototype.toString.call(receiver[key]) === Object.prototype.toString.call(session[key as keyof Session])) {
                Object.assign(updateState, { [key]: receiver[key] });
                continue;
            }
            console.warn(`you just send a invalid data: {${key}: ${receiver[key]}} to update session, please check it`);
        }
        setSession(updateState);
        setTimeout(()=>{
          if (updateState.parseCompleted !== undefined || updateState.clusterCompleted !== undefined || updateState.unitcount !== undefined ) {
            connector.send({
              event: 'updateSession',
              body: {
                parseCompleted: session.parseCompleted,
                clusterCompleted: session.clusterCompleted,
                unitcount: session.unitcount,
                ...updateState
              },
            });
          }
        })
    });

  connector.addListener('getParseStatus', () => {
      connector.send({
        event: 'updateSession',
        body: {
          parseCompleted: session.parseCompleted,
          clusterCompleted: session.clusterCompleted,
          unitcount: session.unitcount,
        },
      });
  });

  connector.addListener('updateHtml', (e) => {
    const { modules, port }: {modules: string[], port: number} = e.data;
    setPort(port);
    modulesConfig.forEach((config, index) => {
      config.attributes.src = window.URL.createObjectURL(
          new Blob(
              [modules[index]],
              { type: "text/html" }
          )
      );
    })
    session.isVscode = false;
    connectRemote({ remote: LOCAL_HOST, port: PORT, dataPath: [] });
  });

  if (!session.isVscode) {
    await connectRemote({ remote: LOCAL_HOST, port: PORT, dataPath: [] });
  }
});

function toggleTab(index: number): void {
    activeModule.value = index;
    connector.send({
        event: 'wakeup',
        body: {},
        to: index,
    });
}

</script>
<template>
    <div class="tab-pane">
        <div class="tab-titles">
            <el-menu class="el-menu-title" mode="horizontal" background-color="var(--color-background)" router>
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
                    v-if="(moduleConfig.isDefault || session.isCluster) && !(session.isVscode)"
                    v-bind={...moduleConfig.attributes}
                    :style="{display:activeModule === index?'block':'none'}"
                    :id="`${moduleConfig.name}`"
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
  --el-menu-hover-bg-color: var(--color-border-hover) !important;
}

.el-menu-item {
    margin-right: 1px;
    text-align: center;
    font-size: 14px;
    font-weight: bold;
    color: var(--treeContent-color) !important;
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
