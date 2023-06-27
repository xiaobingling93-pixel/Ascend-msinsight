import { Table } from './database/table';
import { Client } from './types';
// TO_DO
// import { getTrackId } from './utils/common_util';

// TO_DO should be delete
const database = new Table('./trace_view.db');

/**
 * threadTraces handler
 *
 * @param req ThreadTracesRequest
 * @param client Client
 * @returns threadThraces
 */
export const threadTracesHandler = async (req: ThreadTracesRequest, client?: Client): Promise<Record<string, unknown>> => {
    // TO_DO
    // const traceId = getTrackId(req.threadId, String(req.processId));
    const traceId = -692972522;
    const rawThreadTraces = await database.queryThreadTraceList(traceId, req.startTime, req.endTIme) as RawThreadTrace[];
    const threadTraceList = rawThreadTraces.map(
        item => {
            return {
                name: item.name,
                duration: item.duration,
                startTime: item.timestamp,
                endTime: item.timestamp + item.duration,
                depth: item.depth,
                threadId: req.threadId,
            };
        });
    return {
        data: threadTraceList,
    };
};

type RawThreadTrace = {
    timestamp: number;
    duration: number;
    name: string;
    depth: number;
    trace_id: number;
};

export type ThreadTracesRequest = {
    deviceId: number;
    cardId: number;
    processId: number;
    threadId: number;
    startTime: number;
    endTIme: number;
};

export type ThreadTrace = {
    name: string;
    duration: number;
    startTime: number;
    endTime: number;
    depth: number;
    threadId: number;
};
