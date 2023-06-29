import { createServer } from 'http';
import { WebSocketServer } from 'ws';
import { Handler } from '../types';
import { Connection } from './Connection';

export class InsightServer {
    private readonly _server: ReturnType<typeof createServer>;
    private readonly _ws: WebSocketServer;
    private readonly _connections: Connection[] = [];
    private readonly _port: number;

    constructor(port: number, handlers: Record<string, Handler>) {
        this._port = port;
        this._server = createServer({});
        this._ws = new WebSocketServer({ server: this._server });
        this._ws.on('connection', (ws) => {
            this._connections.push(new Connection(ws, handlers));
        });
    }

    run(): void {
        this._server.listen(this._port);
        console.log('server started on port', this._port, ', waiting for connection...');
    }
}
