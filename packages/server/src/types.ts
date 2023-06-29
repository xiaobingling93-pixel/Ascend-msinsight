export class ShadowSession {
    private readonly placeholder = {};
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
