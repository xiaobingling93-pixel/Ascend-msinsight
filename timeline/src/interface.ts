import { NotificationHandler } from './connection/defs';
import { parseSuccessHandler, parseFailHandler, importRemoteHandler, removeRemoteHandler, setTheme, clusterCompletedHandler, removeSingleRemoteHandler } from './connection/handler';

type InsightInterface<Request extends Record<string, unknown>, Response extends Record<string, unknown>> = {
    request: Request;
    response: Response;
};

export type InterfaceDefs = {
    'chart/cpu': InsightInterface<{ chartId: number }, { data: Array<{ ts: number; value: number }>}>;
};

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    'remote/import': importRemoteHandler,
    'remote/remove': removeRemoteHandler,
    'remote/removeSingle': removeSingleRemoteHandler,
    'parse/success': parseSuccessHandler,
    'parse/fail': parseFailHandler,
    setTheme,
    'parse/clusterCompleted': clusterCompletedHandler,
};
