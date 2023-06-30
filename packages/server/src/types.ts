import { ExtremumTimestamp } from './query/data';

export class ShadowSession {
    private readonly placeholder = {};

    private _extremumTimestamp: ExtremumTimestamp = {
        minTimestamp: Number.MAX_VALUE,
        maxTimestamp: -Number.MAX_VALUE,
    };

    private readonly _importedRankIdSet = new Set<number>();

    get extremumTimestamp(): ExtremumTimestamp {
        return this._extremumTimestamp;
    }

    set extremumTimestamp(value: ExtremumTimestamp) {
        this._extremumTimestamp = value;
    }

    get importedRankIdSet(): Set<number> {
        return this._importedRankIdSet;
    }
};

export interface Client {
    // send a notification message to the client, which may not be triggered by a client request
    notify: (method: string, data: Record<string, unknown>) => void;

    // keeps all necessary states of a session for the client, such as start/end timestamp
    shadowSession: ShadowSession;
}

/**
 * Client request handler.
 *
 * @param reqData request data
 * @param client used for sending extra notification messages to the client
 */
export type Handler = (reqData: any, client: Client) => Promise<Record<string, unknown>>;
