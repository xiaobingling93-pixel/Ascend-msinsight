import { NotificationHandler } from './connection/defs';
import { parseMemoryCompletedHandler, removeRemoteHandler, setTheme, wakeUpHandler, updateSessionHandler } from './connection/handler';

type InsightInterface<Request extends Record<string, unknown>, Response extends Record<string, unknown>> = {
    request: Request;
    response: Response;
};

export type InterfaceDefs = {
    'chart/cpu': InsightInterface<{ chartId: number }, { data: Array<{ ts: number; value: number }>}>;
};

export const NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
    'parse/memoryCompleted': parseMemoryCompletedHandler,
    'remote/remove': removeRemoteHandler,
    updateSession: updateSessionHandler,
    wakeup: wakeUpHandler,
    setTheme,
};
