<script setup lang="ts">
import {ref, onMounted, watch, computed, reactive} from 'vue';
import { request } from '@/centralServer/server';
import connector from '@/connection';
import { type ModuleConfig, modulesConfig } from '@/moduleConfig';
import { useSession, type Session } from '@/stores/session';
import { connectRemote } from '@/centralServer/server';
import { LOCAL_HOST, PORT, type ProjectDirectory, setPort } from '@/centralServer/websocket/defs';
import { useDataSources } from '@/stores/dataSource';
import { console } from '@/utils/console';
import HelpIcon from '@/components/icons/help_icon.vue';
import LangZhIcon from '@/components/icons/lang_zh_icon.vue';
import LangEnIcon from '@/components/icons/lang_en_icon.vue';
import VersionInfo from '@/version_info.json';
import { useLanguage, Languages } from '@/hooks/useLanguage';
import { t } from '@/i18n';
import { localStorageService, LocalStorageKeys } from '@/utils/local-storage';
import useWatchTranslation from '@/hooks/useWatchTranslation';
import SwitchTheme from '@/components/SwitchTheme.vue';
import { useCompareConfig } from '@/stores/compareConfig';
import { assign } from 'lodash';
import { safeJSONParse } from '@/utils';
const { initProjectName } = useDataSources();

type SceneType = 'Default' | 'Cluster' | 'Compute' | 'Jupyter';
const scene = ref<SceneType>('Default');
const activeModule = ref(0);
const moduleRefs = ref<HTMLIFrameElement[] | undefined>();
const { session, setSession } = useSession();
const compareConfig = useCompareConfig();
const showHelpModal = ref(false);

// Ascend-Insight版本信息
const version = ref('UNKNOWN');
// Ascend-Insight最近修改事件
const modifyTime = ref('UNKNOWN');
// 版权年份范围，默认2024年（软件首发年分）
const copyrightYear = ref('2024');
const { language, switchLanguage } = useLanguage();

const isChinese = computed(() => language.value === Languages.ZH);
const [Timeline, Memory, Operator, Summary, Communication, Details, Source] = useWatchTranslation([
    'tabs:Timeline',
    'tabs:Memory',
    'tabs:Operator',
    'tabs:Summary',
    'tabs:Communication',
    'tabs:Details',
    'tabs:Source',
]);

const tabsName = reactive({
  Timeline: Timeline,
  Memory: Memory,
  Operator: Operator,
  Summary: Summary,
  Communication: Communication,
  Details: Details,
  Source: Source,
});

interface ITabName {
  Timeline: string;
  Memory: string;
  Operator: string;
  Summary: string;
  Communication: string;
  Details: string;
  Source: string;
}

function getTabName(name: string) {
  return tabsName[name as keyof ITabName] ?? name;
}

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

async function getModuleConfig() {
    const { configs } : any = await request(
        { remote: LOCAL_HOST, port: PORT }, 'global',
        { command: 'moduleConfig/get', params: {} }
    );
    (configs as string[]).forEach(item => {
        let config : ModuleConfig = { name : '', requestName : '', attributes: {} };
        assign(config, safeJSONParse(item));
        if (config.attributes.src && config.attributes.src !== '' && !config.attributes.src.startsWith('http')) {
            modulesConfig.push(config);
        }
    });
}

onMounted(async () => {
    registerEventListeners();

    if (!session.isVscode) {
        await connectRemote({ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] });
    }

    await getModuleConfig();

    const result: any = await request({ remote: LOCAL_HOST, port: PORT }, 'global', {
      command: 'files/getProjectExplorer',
      params: {}
    });
    await initProjectName(result.projectDirectoryList as ProjectDirectory[]);
});

watch(
    () => [session.isBinary, session.isCluster, session.isIpynb],
    () => {
        if (session.isBinary === null && session.isCluster === null) {
            return;
        }
        updateScene();
    },
);

watch(
    () => [useDataSources().lastDataSource.dataPath, compareConfig.isCompareStatus, compareConfig.compareDataInfo.rankId],
    ()=> {
      const filePathList = useDataSources().lastDataSource.dataPath;
      const projectName = useDataSources().lastDataSource.projectName;
      let curRankId: string;
      if (compareConfig.isCompareStatus) {
        curRankId = compareConfig.compareDataInfo.rankId;
      } else if (projectName.length === 0 || filePathList.length === 0) {
        curRankId = '';
      } else {
        curRankId = compareConfig.getRankIdByProjectInfo(projectName, filePathList[0]);
      }
      connector.send({
        event: 'switchDirectory',
        body: { rankId: curRankId, isCompare: compareConfig.isCompareStatus },
      });
    }
);

function registerEventListeners() {
    connector.resigsterAwaitFetch(async (e) => {
        if (!e.data.remote) {
            e.data.remote = useDataSources().lastDataSource;
        }
        const { remote, args, module, voidResponse } = e.data;
        const result = await request(remote, module, args, voidResponse);
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
            // 1.receiver的字段key在session中存在 2.receiver[key]的类型（例如string、boolean)与session[key]也相同，或者session[key]当前为null
            if (
                validPropKeys.includes(key) &&
                (Object.prototype.toString.call(receiver[key]) ===
                    Object.prototype.toString.call(session[key as keyof Session]) ||
                    session[key as keyof Session] === null)
            ) {
                Object.assign(updateState, { [key]: receiver[key] });
                continue;
            }
            console.warn(
                `you just send a invalid data: {${key}: ${receiver[key]}} to update session, please check it`,
            );
        }
        setSession(updateState);
        setTimeout(() => {
            const isSend =
                (updateState.parseCompleted !== undefined ||
                    updateState.clusterCompleted !== undefined ||
                    updateState.unitcount !== undefined ||
                    updateState.isBinary ||
                    updateState.durationFileCompleted ||
                    updateState.isIpynb ||
                    updateState.ipynbUrl !== '')
                && receiver.broadcast !== false;
            if (isSend) {
                connector.send({
                    event: 'updateSession',
                    body: {
                        parseCompleted: session.parseCompleted,
                        clusterCompleted: session.clusterCompleted,
                        unitcount: session.unitcount,
                        ...updateState,
                    },
                });
            }
        });
    });

    connector.addListener('getParseStatus', (e) => {
        const receiver = e.data.body;
        if (session.isIpynb === true) {
            connector.send({
                event: 'module.reset',
                body: {}
            });
        }
        if (receiver?.request !== undefined) {
            let val = {};
            switch (receiver.request) {
                case 'memoryRankIds':
                    val = {memoryRankIds: session.memoryRankIds};
                    break;
                case 'operatorRankIds':
                    val = {operatorRankIds: session.operatorRankIds};
                    break;
                default:
                    break;
            }
            connector.send({
                event: 'updateSession',
                body: val,
                to: getTabIndex(receiver?.from),
            });
            return;
        }
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
                isIpynb: session.isIpynb,
                ipynbUrl: session.ipynbUrl,
            },
        });
    });

    connector.addListener('updateHtml', (e) => {
        const { modules, port }: { modules: string[]; port: number } = e.data;
        setPort(port);
        modulesConfig.forEach((config, index) => {
            config.attributes.src = window.URL.createObjectURL(
                new Blob([modules[index]], { type: 'text/html' }),
            );
        });
        session.isVscode = false;
        connectRemote({ remote: LOCAL_HOST, port: PORT, projectName: '', dataPath: [] });
    });
    connector.addListener('deleteRank', (e) => {
        const receiver = e.data.body;
        if (!receiver) {
            console.warn('data.body is undefined, please check your params');
            return;
        }
        connector.send({ event: 'deleteRank', body: receiver });
    });

    connector.addListener('switchModule', (e) => {
        const body = e.data.body;
        const switchTo = body?.switchTo;
        const index = modulesConfig.findIndex((tab) => tab.requestName === switchTo);
        if (index > -1 && isShow(modulesConfig[index])) {
            activeModule.value = index;
            if (body.toModuleEvent) {
                connector.send({ event: body.toModuleEvent, to: index, body: body.params });
            }
        }
    });

    connector.addListener('getLanguage', (e) => {
        connector.send({
            event: 'switchLanguage',
            to: e.data.from,
            body: { lang: localStorageService.getItem(LocalStorageKeys.LANGUAGE) || 'enUS' },
            target: 'plugin',
        });
    });

    connector.addListener('pluginMounted', (e) => {
        connector.send({
            event: 'wakeupPlugin',
            data: { url: session.toIframeUrl },
            target: 'plugin',
        });
    });
}

function getTabIndex(name: '') {
    const tabs = modulesConfig.filter(moduleConfig => isShow(moduleConfig) && !session.isVscode);
    return tabs.findIndex(item => item.name === name);
}

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
    } else if (session.isIpynb) {
        scen = 'Jupyter';
    } else {
        scen = 'Default';
    }
    return scen;
}

function isShow(moduleConfig: ModuleConfig): Boolean {
    return Boolean(moduleConfig[`is${scene.value}`]);
}

function getActive(): number {
    const validIndexMap = modulesConfig.reduce((pre, cur, index) => {
      if (isShow(cur)) {
        pre.set(index, cur);
      }
      return pre;
    }, new Map<number, ModuleConfig>());
    const jupyterIndex = findIndexByName(validIndexMap, 'Jupyter');
    if (session.isIpynb && jupyterIndex !== -1) {
      return jupyterIndex;
    } else if (!validIndexMap.has(activeModule.value)) {
      return validIndexMap.entries().next().value[0];
    } else {
      return activeModule.value;
    }
}

function findIndexByName(validIndexMap: Map<number, ModuleConfig>, name: string) : number {
    // 默认值-1，表示要查找的页签不存在
    let result = -1;
    validIndexMap.forEach((value, key) => {
      if (value.name === name) {
        result = key;
      }
    });
    return result;
}

function toggleTab(index: number): void {
    activeModule.value = index;
    connector.send({
            event: 'wakeup',
            body: {},
            to: index,
        });
}

function handleToggleLang(): void {
    const lang = isChinese.value ? Languages.EN : Languages.ZH;
    switchLanguage(lang);
}
</script>
<template>
    <div class="tab-pane">
        <div class="tab-titles">
            <el-menu class="el-menu-title" mode="horizontal" :ellipsis="false" router>
                <template v-for="(moduleConfig, index) in modulesConfig">
                    <el-menu-item
                        :key="`title_${index}_${moduleConfig.name}`"
                        v-if="isShow(moduleConfig)"
                        @click="() => toggleTab(index)"
                        :class="index === activeModule && 'active'"
                    >
                        {{ getTabName(moduleConfig.name) }}
                    </el-menu-item>
                </template>
            </el-menu>

            <div class="right-tool-box">
                <SwitchTheme class="tool-item" />
                <el-tooltip content="中文/English" effect="light">
                    <el-icon class="tool-item" data-testid="switch-lng" @click="handleToggleLang">
                        <LangZhIcon class="icon" v-if="isChinese" />
                        <LangEnIcon class="icon" v-else />
                    </el-icon>
                </el-tooltip>

                <el-icon class="tool-item" data-testid="help-icon" @click="questionIconClickHandler">
                    <HelpIcon class="icon" />
                </el-icon>
            </div>
        </div>
        <div class="tab-body">
            <template v-for="(moduleConfig, index) in modulesConfig">
                <iframe
                    :key="`frame-${index}-${moduleConfig.name}`"
                    v-if="isShow(moduleConfig) && !session.isVscode"
                    v-bind="{ ...moduleConfig.attributes }"
                    :style="{ display: activeModule === index ? 'block' : 'none', background: 'transparent', 'color-scheme': 'light' }"
                    :id="`${moduleConfig.name}`"
                    ref="moduleRefs"
                ></iframe>
            </template>
        </div>
        <el-dialog
            v-model="showHelpModal"
            :title="`${t('About')} MindStudio Insight`"
            width="400"
            :show-close="true"
        >
            <ul class="help-ul">
                <li>{{ t('buildVersion', { version, modifyTime }) }}</li>
                <li>{{ t('copyRight', { copyrightYear }) }}</li>
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
    display: flex;
    align-items: center;
    justify-content: space-between;
    height: var(--header-height);
    background: var(--header-bg-color);
}

.tab-titles .right-tool-box {
    display: flex;
    align-items: center;
    padding-right: 20px;
    font-size: 14px;
}

.tab-titles .right-tool-box .tool-item {
    flex-shrink: 0;
    margin-left: 10px;
    cursor: pointer;
    user-select: none;
    width: 20px;
    height: 20px;
}

.tab-titles .right-tool-box .tool-item .icon {
    width: 100%;
    height: 100%;
}

.el-menu {
    border-bottom: none;
    background-color: transparent;
}

.help-ul {
    padding: 0;
    list-style: none;
}

.help-ul > li {
    margin-bottom: 10px;
    line-height: 1.5em;
}

.tab-body {
    flex-grow: 1;
    height: calc(100% - 40px);
    background: var(--mi-bg-color-dark);
}

iframe {
    width: 100%;
    height: 100%;
    border: 0;
}
</style>
