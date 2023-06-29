import { tableMap } from '../database/tableManager';
import { Client } from '../types';
import { getTrackId } from '../utils/common_util';

/**
 * threadTraces handler
 *
 * @param req ThreadTracesRequest
 * @param client Client
 * @returns threadThraces
 */
export const threadTracesHandler = async (req: ThreadTracesRequest, client: Client): Promise<Record<string, unknown>> => {
    const database = tableMap.get(req.cardId);
    const traceId = getTrackId(req.threadId, String(req.processId));
    const threadTraceList = await database?.queryThreadTraceList(client, req.threadId, traceId, req.startTime, req.endTime);
    return {
        data: threadTraceList,
    };
};

/**
 * chart threadTraces req
 */
export type ThreadTracesRequest = {
    deviceId: number;
    cardId: number;
    processId: number;
    threadId: number;
    startTime: number;
    endTime: number;
};

/**
 * database single data type
 */
export type RowThreadTrace = {
    id: number;
    start_time: number;
    duration: number;
    name: string;
    depth: number;
    trace_id: number;
};

/**
 * respnose data type
 */
export type ThreadTrace = {
    name: string;
    duration: number;
    startTime: number;
    endTime: number;
    depth: number;
    threadId: number;
};
