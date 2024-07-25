/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import i18n from 'lib/i18n';
import { notification } from 'lib/components';
import './DragFile.css';
import { formatTimestamp } from '../../utils/humanReadable';
import { logger } from '../../utils/Logger';
import connector from '../../connection';
import type { NotificationHandler } from '../../connection/defs';

const isSupportCompress = (window as any).CompressionStream !== undefined;
const MAX_FILE_SIZE = 10 * 1024 * 1024 * 1024; // 10G
const DEFAULT_SLICE_SIZE = isSupportCompress ? 10 * 1024 * 1024 : 1024 * 1024; // 若支持压缩，切片为10M，否则为1M
const ALLOW_FILE_TYPES = ['application/json'];

export interface FileDataType {
    data?: any;
    attr: {
        [prop: string]: any;
        name: string; // 文件名
        path: string; // 文件路径
        isInFolder?: boolean;
        index?: number;
        count?: number;
        isLast?: boolean;
    };
    succeed?: boolean;
    res?: any;
    text?: string;
    error?: string | unknown;
}
export interface CheckResultType {
    usable: boolean;
    files?: FileDataType[];
    totalSize?: number;
    error?: string;
}

interface SuccessHandler {
    (res: any): void;
}

interface DropHandler {
    (res: CheckResultType): void;
}

interface DragFileParams {
    id?: string;
    onSuccess?: SuccessHandler;
    onDrop?: DropHandler;
}

interface ImportFileData {
    isCluster: boolean;
    reset: boolean;
    result: Array<{
        cardName: string;
        rankId: string;
        cardPath: string;
        result: true;
    }>;
}
const regexArrayFormat = /^\[\{.*"ph":/;
const regexObjectFormat = /^\{.*"traceEvents": /;
class DragFile {
    onSuccess?: SuccessHandler;
    onDrop?: DropHandler;
    protected _stopped = false;
    protected _processing = false;
    constructor({ id, onSuccess, onDrop }: DragFileParams) {
        const dropZone = id !== undefined ? document.getElementById(id) : document;
        if (dropZone === null) {
            return;
        }

        this.addEvent(dropZone);
        // 如果页面中有iframe
        if (dropZone.querySelectorAll('iframe').length !== 0) {
            dropZone.querySelectorAll('iframe').forEach((frame: HTMLIFrameElement) => {
                if (frame.contentWindow !== null) {
                    this.addEvent(frame.contentWindow);
                }
            });
        }
        this.onSuccess = onSuccess;
        this.onDrop = onDrop;
    }

    get isStopped(): boolean {
        return this._stopped;
    }

    get isProcessing(): boolean {
        return this._processing;
    }

    // eslint-disable-next-line @typescript-eslint/adjacent-overload-signatures
    set isStopped(val: boolean) {
        this._stopped = val;
    }

    // eslint-disable-next-line @typescript-eslint/adjacent-overload-signatures
    set isProcessing(val: boolean) {
        this._processing = val;
    }

    addEvent(dropZone: HTMLElement | Document | Window | null): void {
        if (dropZone === null) {
            return;
        }
        dropZone.addEventListener('dragenter', (e: any) => {
            e.preventDefault();
            e.stopPropagation();
        }, false);

        dropZone.addEventListener('dragover', (e: any) => {
            e.preventDefault();
            e.stopPropagation();
            e.target.classList.add('files-hover');
        }, false);

        dropZone.addEventListener('dragleave', (e: any) => {
            e.preventDefault();
            e.stopPropagation();
            e.target.classList.remove('files-hover');
        }, false);
        // 处理文件
        dropZone.addEventListener('drop', (e: any) => {
            e.preventDefault();
            e.stopPropagation();
            e.target.classList.remove('files-hover');
            this.handleDrop(e);
        }, false);
    }

    async handleDrop(e: any): Promise<void > {
        if (this.isProcessing) {
            notify('Processing');
            return;
        }
        this.isStopped = false;
        // 1.读取文件
        const allFiles = await getFilelistByItems(e.dataTransfer.items);
        // 2.检查文件
        const fileBag = await this.checkFiles(allFiles);
        if (!fileBag.usable) {
            notify('CheckError', fileBag.error);
            return;
        }

        if (this.onDrop !== undefined) {
            this.onDrop(fileBag);
        }
        // 3.处理文件
        this.isProcessing = true;
        await this.handleFile(fileBag);
        this.isProcessing = false;
    }

    async handleFile(fileBag: CheckResultType): Promise<any> {
        return fileBag;
    }

    async checkFiles(list: FileDataType[]): Promise<CheckResultType> {
        const files = list;
        let totalSize = 0;
        files.forEach((file: FileDataType, index: number) => {
            file.attr.index = index + 1;
            file.attr.count = files.length;
            file.attr.isLast = (index + 1) === files.length;
            file.attr.size = file.data.size;
            totalSize += file.data.size;
        });
        return {
            usable: true,
            totalSize,
            files,
        };
    }
}

class DragFileImport extends DragFile {
    NOTIFICATION_HANDLERS: Record<string, NotificationHandler> = {
        'remote/remove': this.removeRemoteHandler,
        'remote/removeSingle': this.removeSingleRemoteHandler,
    };

    uploadingFileName: string = '';
    constructor(params: any) {
        super(params);
        addListeners(this.NOTIFICATION_HANDLERS, this);
    }

    async checkFile(file: FileDataType): Promise<CheckResultType> {
        if (!this.checkFileSize(file.data.size)) {
            return {
                usable: false,
                error: i18n.t('timeline:dragFile:FileSizeLimitWarning'),
            };
        }

        const isValidFormat = await this.checkFileFormat(file);
        if (!isValidFormat) {
            return {
                usable: false,
                error: i18n.t('timeline:dragFile:FileFormatError'),
            };
        }

        const timestamp = formatTimestamp(Date.now(), 'MM.DD HH:mm:ss');
        file.attr.name = `${file.attr.name}(${timestamp})`;
        file.attr.path = `${file.attr.path}(${timestamp})`;
        this.uploadingFileName = file.attr.name;
        return {
            usable: true,
            totalSize: file.data.size,
            files: [file],
        };
    }

    async checkFiles(list: FileDataType[]): Promise<CheckResultType> {
        const singleFiles = list.filter((file: FileDataType) => !file.attr.isInFolder);
        if (list.length === 1 && singleFiles.length === 1) {
            return await this.checkFile(singleFiles[0]);
        } else {
            return {
                usable: false,
                error: i18n.t('timeline:dragFile:OneFileOnlyWarning'),
            };
        }
    }

    checkFileFormat(file: FileDataType): Promise<boolean> {
        return new Promise((resolve, reject): void => {
            if (ALLOW_FILE_TYPES.includes(file.data.type)) {
                const reader = new FileReader();
                reader.readAsText(file.data.slice(0, 10 * 1024));
                reader.onload = (event): void => {
                    const text: any = event.target?.result;
                    resolve(this.isTraceViewFormat(text));
                };
                reader.onerror = (): void => {
                    resolve(false);
                };
            } else {
                resolve(false);
            }
        });
    }

    isTraceViewFormat(text: string): boolean {
        // 校验TraceView文件格式
        if (regexArrayFormat.test(text)) {
            return true;
        }

        if (regexObjectFormat.test(text)) {
            // 提前检查是否包含 "traceEvents":
            const traceEventsIndex = text.indexOf('"traceEvents": ');
            if (traceEventsIndex !== -1) {
                const traceEventsVal = text.slice(traceEventsIndex + '"traceEvents": '.length);
                return regexArrayFormat.test(traceEventsVal);
            }
        }

        return false;
    }

    checkFileSize(size: number): boolean {
        return size <= MAX_FILE_SIZE;
    }

    async handleFile(fileBag: CheckResultType): Promise<any> {
        return await this.upload(fileBag);
    }

    async upload(fileBag: CheckResultType): Promise<any> {
        const { files = [] } = fileBag;
        return await this.loopUpload(files);
    }

    async loopUpload(list: FileDataType[]): Promise<any> {
        const resList = [];
        for (let i = 0; i < list.length; i++) {
            const res = await this.readFile(list[i]);
            resList.push(res);
            if (this.onSuccess !== undefined) {
                this.onSuccess(res.res);
            }
        }
        return resList;
    }

    async loadFile(fileBlob: Blob, file: FileDataType, i: number, count: number): Promise<FileDataType> {
        const { attr } = file;
        const text: Blob = fileBlob;
        if (text !== null && text !== undefined) {
            const isLastSlice = count <= 1 || i === count;
            let compressSlice;

            try {
                if (isSupportCompress) {
                    compressSlice = await compressData(text);
                } else {
                    compressSlice = await blobToArrayBuffer(text);
                }
                const res: ImportFileData = await window.requestData({
                    command: 'upload/file',
                    keepRawData: true,
                    bufferField: 'text',
                    params: {
                        text: compressSlice,
                        textLength: compressSlice.byteLength,
                        fileAttr: attr,
                        slice: {
                            isSliced: count > 1, index: i, count, isLast: isLastSlice,
                        },
                    },
                    module: 'timeline',
                    voidResponse: !isLastSlice,
                });
                const isSuccess = res?.result?.[0].result;
                return { succeed: isSuccess, attr, res };
            } catch (error) {
                return { succeed: false, attr, error };
            }
        } else {
            return { succeed: false, attr, error: 'Blob is null or undefined' };
        }
    }

    async readFile(file: FileDataType): Promise<FileDataType> {
        const slicesize = DEFAULT_SLICE_SIZE;
        const { data } = file;
        const count = Math.ceil(data.size / slicesize);
        let lastRes: FileDataType = { succeed: false, attr: file.attr };

        for (let i = 1; i <= count; i++) {
            const start = (i - 1) * slicesize;
            const end = Math.min(data.size, i * slicesize);
            const fileBlob = data.slice(start, end);
            if (i === count) {
                lastRes = await this.loadFile(fileBlob, file, i, count);
            } else {
                this.loadFile(fileBlob, file, i, count);
            }
            if (i % 20 === 0) {
                await sleep(1200);
            }
            if (this.isStopped) {
                break;
            }
        }

        return lastRes;
    }

    removeRemoteHandler(): void {
        this.isStopped = true;
    }

    removeSingleRemoteHandler(data: any): void {
        if (data.singleDataPath === this.uploadingFileName) {
            this.isStopped = true;
        }
    }
}

function blobToArrayBuffer(blob: Blob): Promise<ArrayBuffer> {
    return new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onload = (): void => {
            resolve(reader.result as ArrayBuffer);
        };
        reader.onerror = reject;
        reader.readAsArrayBuffer(blob);
    });
}

async function compressData(input: Blob): Promise<ArrayBuffer> {
    const compressedChunks: Uint8Array[] = [];
    const compressor = new (window as any).CompressionStream('deflate');
    const writableStream = new WritableStream<Uint8Array>({
        write(chunk): void {
            compressedChunks.push(chunk);
        },
    });

    const readableStream = input.stream();

    await readableStream.pipeThrough(compressor).pipeTo(writableStream);

    const concatenated = new Uint8Array(
        compressedChunks.reduce(
            (acc: number[], chunk: Uint8Array) => acc.concat(Array.from(chunk)),
            [],
        ),
    );

    return concatenated.buffer;
}

function sleep(duration = 1000): Promise<void> {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve();
        }, duration);
    });
}

async function getFilelistByItems(items: DataTransferItemList): Promise<any> {
    return new Promise((resolve, reject) => {
        const list = [];
        for (const item of items) {
            if (item.kind === 'file') {
                const entry = item.webkitGetAsEntry();
                list.push(getFileFromEntryRecusively(entry));
            }
        }
        Promise.all(list).then(values => {
            const files = values.flat(1);
            files.forEach((file: FileDataType, index: number) => {
                file.attr.index = index + 1;
                file.attr.count = files.length;
                file.attr.isLast = (index + 1) === files.length;
                file.attr.isInFolder = file.attr.path !== file.attr.name;
            });
            resolve(files);
        });
    });
}

function getFileFromEntryRecusively(entry: any): Promise<any> {
    return new Promise((resolve, reject) => {
        if (entry.isFile === true) {
            entry.file((data: any) => {
                const fileInfo = { data, attr: { path: entry.fullPath.slice(1), name: data.name } };
                resolve(fileInfo);
            }, (e: any) => {
                logger('ReadFile', e);
            });
        } else {
            const reader = entry.createReader();
            readEntries(resolve, reader);
        }
    });
}

function readEntries(resolve: any, reader: any, list: any = []): void {
    reader.readEntries(
        (entries: any) => {
            list.push(...entries.map((entry: any) => getFileFromEntryRecusively(entry)));
            if (entries.length === 0 || entries.length < 100) {
                Promise.all(list).then(values => {
                    resolve(values.flat(1));
                });
            } else {
                readEntries(resolve, reader, list);
            }
        },
        (e: any) => {
            logger('ReadFile', e);
        },
    );
}

type NotifyType = 'Processing' | 'CheckError';
function notify(type: NotifyType, param?: any): void {
    switch (type) {
        case 'CheckError':
            notification.error({ className: 'drag-file-hit', message: i18n.t('timeline:dragFile:Error'), description: param, placement: 'top' });
            break;
        case 'Processing':
            notification.error({
                className: 'drag-file-hit',
                message: i18n.t('timeline:dragFile:Warn'),
                description: i18n.t('timeline:dragFile:FileIsProcessingWarning'),
                duration: 5,
                placement: 'top',
            });
            break;
        default:
            notification[type](param);
            break;
    }
};

export const dragFileImportInit = ({ id, onSuccess, onDrop }: DragFileParams): DragFileImport => {
    return new DragFileImport({ id, onSuccess, onDrop });
};

const addListeners = (handlers: Record<string, NotificationHandler>, _this: DragFileImport): void => {
    Object.entries(handlers).forEach(([event, callback]) => {
        connector.addListener(event, (e: MessageEvent<{ event: string; body: Record<string, unknown> }>) => {
            const res = e.data;
            if (typeof res.body !== 'object') {
                return;
            }
            callback.apply(_this, [res.body]);
        });
    });
};
