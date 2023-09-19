import { NotificationHandler } from './connection/defs';
import {
    parseSuccessHandler, parseFailHandler, removeRemoteHandler,
    parseClusterSuccessHandler, setTheme,
} from './connection/handler';

type InsightInterface<Request extends Record<string, unknown>, Response extends Record<string, unknown>> = {
    request: Request;
    response: Response;
};

export type InterfaceDefs = {
    'chart/cpu': InsightInterface<{ chartId: number }, { data: Array<{ ts: number; value: number }>}>;
};

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    'remote/remove': removeRemoteHandler,
    'parse/success': parseSuccessHandler,
    'parse/fail': parseFailHandler,
    setTheme,
    'parse/clusterCompleted': parseClusterSuccessHandler,
};
