/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
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
