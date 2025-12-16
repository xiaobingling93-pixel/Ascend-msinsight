/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
import type { NotificationHandler } from './connection/defs';
import {
    removeRemoteHandler,
    setTheme,
    updateSessionHandler,
    locateCommunication,
    switchLanguageHandler,
    updateCommunicatorDataHandler,
    baselineToggleHandler,
    profilingExpertDataParsedHandler,
    switchDirectoryHandler,
    frameLoadedHandler,
    updateClusterPageInfoHandler,
    viewCommunicationDurationAnalysisHandler,
} from './connection/handler';

interface InsightInterface<Request extends Record<string, unknown>, Response extends Record<string, unknown>> {
    request: Request;
    response: Response;
}

export interface InterfaceDefs {
    'chart/cpu': InsightInterface<{ chartId: number }, { data: Array<{ ts: number; value: number }>}>;
}

const HEATMAP_COMPLETED = 'parse/heatmapCompleted';

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler<any>> = {
    'frame/loaded': frameLoadedHandler,
    'remote/remove': removeRemoteHandler,
    'remote/reset': removeRemoteHandler,
    'module.reset': removeRemoteHandler,
    setTheme,
    updateClusterPageInfo: updateClusterPageInfoHandler,
    updateSession: updateSessionHandler,
    locateCommunication,
    viewCommunicationDurationAnalysis: viewCommunicationDurationAnalysisHandler,
    switchLanguage: switchLanguageHandler,
    updateCommunicatorData: updateCommunicatorDataHandler,
    clusterBaselineToggle: baselineToggleHandler,
    [HEATMAP_COMPLETED]: profilingExpertDataParsedHandler,
    switchDirectory: switchDirectoryHandler,
};
