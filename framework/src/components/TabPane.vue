<script setup lang="ts">
import {ref, onMounted, watch} from 'vue';
import {request} from '@/centralServer/server';
import connector from '@/connection';
import {type ModuleConfig, modulesConfig} from '@/moduleConfig';
import {useSession, type Session} from '@/stores/session';
import {connectRemote} from '@/centralServer/server';
import {LOCAL_HOST, PORT, setPort} from '@/centralServer/websocket/defs';
import {useDataSources} from '@/stores/dataSource';
import {Console} from '@/utils/console';
import HelpIcon from '@/components/icons/help_icon.vue';
import VersionInfo from '@/version_info.json';
import {useResource} from '@/stores/resourceComp';
const { confirmDrop } = useDataSources();
const { setCurrentPath } = useResource();

type SceneType = 'Default' | 'Cluster' | 'Compute';
const scene = ref<SceneType>('Default');
const activeModule = ref(0);
const moduleRefs = ref<HTMLIFrameElement[] | undefined>();
const {session, setSession} = useSession();
const showHelpModal = ref(false);

// Ascend-Insight版本信息
const version = ref('UNKNOWN');
// Ascend-Insight最近修改事件
const modifyTime = ref('UNKNOWN');
// 版权年份范围，默认2024年（软件首发年分）
const copyrightYear = ref('2024');

/**
 * 问号图标鼠标点击事件处理函数
 *
 * @param e 鼠标事件
 */
function questionIconClickHandler(e: MouseEvent) {
  // 读取文件内容
  version.value = VersionInfo.version;
  modifyTime.value = VersionInfo.modifyTime;

  // “软件最近修改年份”不是“首次发布年份时”，版权时间范围需要修改为“首年年份-最近修改年份”
  const modifyYear = VersionInfo.modifyTime.split('/')[0];
  if (modifyYear !== copyrightYear.value) {
    copyrightYear.value = copyrightYear.value + '-' + modifyYear;
  }

  e.stopPropagation();
  // 显示“帮助”弹框，展示版本信息内容
  showHelpModal.value = true;
}


onMounted(async () => {
    connector.resigsterAwaitFetch(async (e) => {
        if (!e.data.remote) {
            e.data.remote = useDataSources().lastDataSource;
        }
        const {remote, args, module, voidResponse} = e.data;
        const result = await request(remote, module, args, voidResponse);
        return {dataSource: remote, body: result};
    });

    connector.addListener('updateSession', (e) => {
        const receiver = e.data.body;
        if (!receiver) {
            Console.warn('data.body is undefined, please check your params');
            return;
        }
        const receivePropKeys = Object.keys(receiver);
        const validPropKeys = Object.keys(session);
        const updateState: any = {};
        for (const key of receivePropKeys) {
            // 1.receiver的字段key在session中存在 2.receiver[key]的类型（例如string、boolean)与session[key]也相同，或者session[key]当前为null
            if (validPropKeys.includes(key) && (Object.prototype.toString.call(receiver[key]) === Object.prototype.toString.call(session[key as keyof Session]) ||
                session[key as keyof Session] === null)) {
                Object.assign(updateState, {[key]: receiver[key]});
                continue;
            }
            Console.warn(`you just send a invalid data: {${key}: ${receiver[key]}} to update session, please check it`);
        }
        setSession(updateState);
        setTimeout(() => {
            const isSend = updateState.parseCompleted !== undefined || updateState.clusterCompleted !== undefined || updateState.unitcount !== undefined
                || updateState.isBinary || updateState.durationFileCompleted;
            if (isSend) {
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
        });
    });

    connector.addListener('getParseStatus', () => {
        connector.send({
            event: 'updateSession',
            body: {
                parseCompleted: session.parseCompleted,
                clusterCompleted: session.clusterCompleted,
                isFullDb: session.isFullDb,
                unitcount: session.unitcount,
                coreList: session.coreList,
                sourceList: session.sourceList,
                durationFileCompleted: session.durationFileCompleted,
            },
        });
    });

    connector.addListener('updateHtml', (e) => {
        const {modules, port}: { modules: string[], port: number } = e.data;
        setPort(port);
        modulesConfig.forEach((config, index) => {
            config.attributes.src = window.URL.createObjectURL(
                new Blob(
                    [modules[index]],
                    {type: 'text/html'}
                )
            );
        });
        session.isVscode = false;
        connectRemote({remote: LOCAL_HOST, port: PORT, dataPath: []});
    });
    connector.addListener('deleteRank', (e) => {
        const receiver = e.data.body;
        session.loading = false;
        if (!receiver) {
            Console.warn('data.body is undefined, please check your params');
            return;
        }
        connector.send({event: 'deleteRank', body: receiver});
    });

    connector.addListener('dropFile', (e) => {
      const res = e.data.body;
      const path = res.result?.[0]?.cardPath;
      const dataSource = { remote: LOCAL_HOST, port: PORT, dataPath: [path] };
      confirmDrop(dataSource, res);
    });

    if (!session.isVscode) {
        await connectRemote({remote: LOCAL_HOST, port: PORT, dataPath: []});
    }
});
watch(() => [session.isBinary, session.isCluster], () => {
    if (session.isBinary === null && session.isCluster === null) {
        return;
    }
    updateScene();
});

function updateScene() {
    scene.value = getScene();
    activeModule.value = getActive();
}

function getScene(): SceneType {
    let scen: SceneType;
    if (session.isBinary) {
        scen = 'Compute';
    } else if (session.isCluster) {
        scen = 'Cluster';
    } else {
        scen = 'Default';
    }
    return scen;
}

function isShow(moduleConfig: ModuleConfig): Boolean {
    return Boolean(moduleConfig[`is${scene.value}`]);
}

function getActive(): number {
    const validIndexlist = modulesConfig.reduce((pre, cur, index) => {
        if (isShow(cur)) {
            pre.push(index);
        }
        return pre;
    }, [] as number[]);
    if (!validIndexlist.includes(activeModule.value)) {
        return validIndexlist[0];
    }
    return activeModule.value;
}

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
                <template v-for="(moduleConfig, index) in modulesConfig">
                    <el-menu-item
                        :key="`title_${index}_${moduleConfig.name}`"
                        v-if="isShow(moduleConfig)"
                        @click="() => toggleTab(index)"
                        :class="index === activeModule && 'active'"
                    >
                        {{ moduleConfig.name }}
                    </el-menu-item>
                </template>
            </el-menu>
            <el-icon class="QuestionIcon" @click="questionIconClickHandler" background-color="var(--color-background)" router>
              <HelpIcon style="height: 30px; width: 30px; cursor: pointer;"/>
            </el-icon>
        </div>
        <div class="tab-body">
            <template v-for="(moduleConfig, index) in modulesConfig">
                <iframe
                    :key="`frame-${index}-${moduleConfig.name}`"
                    v-if="isShow(moduleConfig) && !session.isVscode"
                    v-bind={...moduleConfig.attributes}
                    :style="{display:activeModule === index?'block':'none'}"
                    :id="`${moduleConfig.name}`"
                    ref="moduleRefs"
                ></iframe>
            </template>
        </div>
        <el-dialog v-model="showHelpModal" title="About Ascend Insight" width="20%" :close-on-click-modal="false" :show-close="true">
          <ul class="help-ul">
            <li>
              Build {{ version.valueOf() }}, build on {{ modifyTime.valueOf() }}
            </li>
            <li>
              Copyright © {{ copyrightYear.valueOf() }} Huawei Technologies Co, Ltd. All Rights Reserved.
            </li>
          </ul>
        </el-dialog>
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
    display: inline-flex;
    border: none;
    height: 30px;
    width: 95%;
    line-height: 30px;
    border-bottom: none !important;
    --el-menu-hover-bg-color: var(--color-border-hover) !important;
}

.QuestionIcon {
  display: inline-flex;
  width: 5%;
  height: 30px;
  position: absolute;
  right: 0;
}

.help-ul {
  list-style: none;
}

.help-ul > li {
  margin-bottom: 10px;
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
    width: calc(100% - 60px);
    height: 1px;
    background-color: #007aff;
    left:30px;
}

@media (min-height: 1024px) {
    .tab-titles {
        height: 40px;
    }
}

.tab-body {
    flex-grow: 1;
    height: calc(100% - 40px);
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
