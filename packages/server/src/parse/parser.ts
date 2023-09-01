import fs from 'fs';
import { Table } from '../database/table';
import { getTrackId } from '../utils/common_util';
import { getLoggerByName } from '../logger/loggger_configure';

export const logger = getLoggerByName('parser', 'info');
export class Parser {
    private counter = 0;
    private ignoreCount = 0;
    private readonly fd: number;
    private readonly table: Table;

    constructor(filePath: string, dbPath: string) {
        this.table = new Table(dbPath);
        this.fd = fs.openSync(filePath, 'r');
    }

    public async parse(readPosition: number, readSize: number): Promise<void> {
        await this.table.setConfig();
        const buf = Buffer.alloc(readSize);
        fs.readSync(this.fd, buf, 0, readSize, readPosition);
        const str = buf.toString('utf-8');
        let events: any;
        try {
            events = JSON.parse('[' + str + ']');
        } catch (e) {
            logger.info(`parse json error. ${e} \njson string: ${str.slice(0, 100)}......${str.slice(-100, -1)}`);
            return;
        }
        for (const event of events) {
            await this.handler(event);
        }
    }

    // 缓存入库
    public async parseEnd(): Promise<void> {
        fs.closeSync(this.fd);
        await this.table.commitData();
        await this.table.close();
    }

    public getCount(): number {
        return this.counter;
    }

    public getIgnoreCount(): number {
        return this.ignoreCount;
    }

    async handler(data: any): Promise<void> {
        ++this.counter;
        if (!('ph' in data)) {
            ++this.ignoreCount;
            return;
        }
        switch (data.ph) {
            case 'M':
                this.metadataEventsHandler(data);
                break;
            case 'X':
                await this.CompleteEventsHandler(data);
                break;
            case 's':
                await this.flowEventsHandler(data);
                break;
            case 'f':
                await this.flowEventsHandler(data);
                break;
            default:
                ++this.ignoreCount;
        }
    }

    metadataEventsHandler(data: { name: string; tid: number; pid: string; args: any }): void {
        switch (data.name) {
            case 'process_name':
                this.table.updateProcessName(data.pid, data.args.name);
                break;
            case 'thread_name':
                this.table.updateThreadName(getTrackId(data.tid, data.pid), data.tid, data.pid, data.args.name);
                break;
            case 'process_labels':
                this.table.updateProcessLabel(data.pid, data.args.labels);
                break;
            case 'process_sort_index':
                this.table.updateProcessSortIndex(data.pid, data.args.sort_index);
                break;
            case 'thread_sort_index':
                this.table.updateThreadSortIndex(getTrackId(data.tid, data.pid), data.args.sort_index);
                break;
            default:
                ++this.ignoreCount;
        }
    }

    async CompleteEventsHandler(data: any): Promise<void> {
        data.track_id = getTrackId(data.tid, data.pid);
        await this.table.insertSlice(data);
    }

    async flowEventsHandler(data: any): Promise<void> {
        data.track_id = getTrackId(data.tid, data.pid);
        await this.table.insertFlow(data);
    }
}
