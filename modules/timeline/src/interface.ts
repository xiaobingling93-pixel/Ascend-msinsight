import { NotificationHandler } from './connection/defs';
import {
    parseSuccessHandler,
    parseProgressHandler,
    parseFailHandler,
    importRemoteHandler,
    removeRemoteHandler,
    setTheme,
    clusterCompletedHandler,
    removeSingleRemoteHandler,
    clusterDurationCompletedHandler,
    dragImportSuccessHandler,
    locateUnitHandler,
    jupyterCompletedHandler,
    switchLanguageHandler,
    parseOperatorSuccessHandler,
    parseMemorySuccessHandler,
} from './connection/handler';

const JUPYTER_COMPLETED = 'parse/jupyterCompleted';
type InsightInterface<Request extends Record<string, unknown>, Response extends Record<string, unknown>> = {
    request: Request;
    response: Response;
};

export type InterfaceDefs = {
    'chart/cpu': InsightInterface<{ chartId: number }, { data: Array<{ ts: number; value: number }>}>;
};

const MEMORY_COMPLETED = 'parse/memoryCompleted';
const OPERATOR_COMPLETED = 'parse/operatorCompleted';
export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    'remote/import': importRemoteHandler,
    'remote/remove': removeRemoteHandler,
    'remote/reset': removeRemoteHandler,
    'remote/removeSingle': removeSingleRemoteHandler,
    'parse/success': parseSuccessHandler,
    'parse/progress': parseProgressHandler,
    'parse/fail': parseFailHandler,
    setTheme,
    'parse/clusterCompleted': clusterCompletedHandler,
    'parse/clusterStep2Completed': clusterDurationCompletedHandler,
    'drag/import': dragImportSuccessHandler,
    locateUnit: locateUnitHandler,
    [JUPYTER_COMPLETED]: jupyterCompletedHandler,
    switchLanguage: switchLanguageHandler,
    [MEMORY_COMPLETED]: parseMemorySuccessHandler,
    [OPERATOR_COMPLETED]: parseOperatorSuccessHandler,
};
