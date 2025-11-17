/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
import type { NotificationHandler } from './connection/defs';
import {
    parseSuccessHandler,
    parseProgressHandler,
    parseFailHandler,
    allSuccessHandler,
    importRemoteHandler,
    savePageSettingRemoteHandler,
    removeRemoteHandler,
    setTheme,
    clusterCompletedHandler,
    removeSingleRemoteHandler,
    clusterDurationCompletedHandler,
    locateUnitHandler,
    switchLanguageHandler,
    baselineAddHandler,
    removeBaselineHandler,
    updateProjectNameHandler,
    resetRemoteHandler, findBlock,
    parseUnitCompletedHandler,
} from './connection/handler';

interface InsightInterface<Request extends Record<string, unknown>, Response extends Record<string, unknown>> {
    request: Request;
    response: Response;
};

const CHART_CPU = 'chart/cpu';
export interface InterfaceDefs {
    [CHART_CPU]: InsightInterface<{ chartId: number }, { data: Array<{ ts: number; value: number }>}>;
};

const REMOTE_SAVE_PAGE_SETTING = 'remote/savePageSetting';
const REMOTE_IMPORT = 'remote/import';
const REMOTE_REMOVE = 'remote/remove';
const REMOTE_RESET = 'remote/reset';
const REMOTE_REMOVE_SINGLE = 'remote/removeSingle';
const PARSE_SUCCESS = 'parse/success';
const PARSE_PROGRESS = 'parse/progress';
const PARSE_FAIL = 'parse/fail';
const PARSE_CLUSTER_COMPLETED = 'parse/clusterCompleted';
const PARSE_CLUSTER_STEP2_COMPLETED = 'parse/clusterStep2Completed';
const BASELINE_ADD = 'baseline/add';
const BASELINE_REMOVE = 'baseline/remove';
const PARSE_UNIT_COMPLETED = 'parse/unitCompleted';
export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    [REMOTE_SAVE_PAGE_SETTING]: savePageSettingRemoteHandler,
    [REMOTE_IMPORT]: importRemoteHandler,
    [REMOTE_REMOVE]: removeRemoteHandler,
    [REMOTE_RESET]: resetRemoteHandler,
    allPagesSuccess: allSuccessHandler,
    [REMOTE_REMOVE_SINGLE]: removeSingleRemoteHandler,
    [PARSE_SUCCESS]: parseSuccessHandler,
    [PARSE_PROGRESS]: parseProgressHandler,
    [PARSE_FAIL]: parseFailHandler,
    [PARSE_CLUSTER_COMPLETED]: clusterCompletedHandler,
    [PARSE_CLUSTER_STEP2_COMPLETED]: clusterDurationCompletedHandler,
    [BASELINE_ADD]: baselineAddHandler,
    [BASELINE_REMOVE]: removeBaselineHandler,
    setTheme,
    locateUnit: locateUnitHandler,
    switchLanguage: switchLanguageHandler,
    updateProjectName: updateProjectNameHandler,
    findBlock,
    [PARSE_UNIT_COMPLETED]: parseUnitCompletedHandler,
};
