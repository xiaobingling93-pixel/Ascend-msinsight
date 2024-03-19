/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
import React from 'react';
import { notification } from 'antd';
import './DragFile.css';
import { formatTimestamp } from '../../utils/humanReadable';
import { Logger } from '../../utils/Logger';

const MAX_FILE_SIZE = 10 * 1024 * 1024 * 1024; // 10G
const DEFAULT_SLICE_SIZE = 1024 * 1024; // Byte

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

class DragFile {
    onSuccess?: SuccessHandler;
    onDrop?: DropHandler;
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

    get isProcessing(): boolean {
        return this._processing;
    }

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
    async checkFile(file: FileDataType): Promise<CheckResultType> {
        if (!this.checkFileSize(file.data.size)) {
            return {
                usable: false,
                error: 'Please select a file smaller than 10GB',
            };
        }

        const isValidFormat = await this.checkFileFormat(file);
        if (!isValidFormat) {
            return {
                usable: false,
                error: 'File format error: Only Trace Event Format is supported',
            };
        }

        const timestamp = formatTimestamp(Date.now(), 'MMDDHHmmss.SSS');
        file.attr.name = `${file.attr.name}(${timestamp})`;
        file.attr.path = `${file.attr.path}(${timestamp})`;
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
                error: 'Only Allow one File',
            };
        }
    }

    checkFileFormat(file: FileDataType): Promise<boolean> {
        return new Promise((resolve, reject): void => {
            const reader = new FileReader();
            reader.readAsText(file.data.slice(0, 10 * 1024));
            reader.onload = (event): void => {
                const text: any = event.target?.result;
                resolve(this.isTraceViewFormat(text));
            };
            reader.onerror = (): void => {
                resolve(false);
            };
        });
    }

    isTraceViewFormat(text: string): boolean {
        // 校验TraceView文件格式
        const regexArrayFormat = /^\[\{.*"ph":/;
        const regexObjectFormat = /^\{.*"traceEvents": /;
        if (regexArrayFormat.test(text)) {
            return true;
        } else if (regexObjectFormat.test(text)) {
            const traceEventsVal = text.split('"traceEvents": ')[1];
            return regexArrayFormat.test(traceEventsVal);
        } else {
            return false;
        }
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

    loadFile(fileBlob: Blob, file: FileDataType, i: number, count: number, resolve: (value: FileDataType) => void): void {
        const { attr } = file;
        const reader = new FileReader();
        reader.readAsText(fileBlob);
        reader.onload = (event): void => {
            const text: any = event.target?.result;
            if (text !== null && text !== undefined) {
                const isLastSlice = count <= 1 || i === count;
                try {
                    window.requestData('upload/file', {
                        text,
                        fileAttr: attr,
                        slice: {
                            isSliced: count > 1, index: i, count, isLast: isLastSlice,
                        },
                    }, 'timeline', !isLastSlice).then((res: any) => {
                        resolve({ succeed: true, attr, res });
                    }).catch((error: any) => {
                        resolve({ succeed: false, attr, error });
                    });
                } catch (error) {
                    resolve({ succeed: false, attr, error });
                }
            }
        };
        reader.onerror = (): void => {
            resolve({ succeed: false, attr });
        };
    }

    readFile(file: FileDataType): Promise<any> {
        const slicesize = DEFAULT_SLICE_SIZE;
        return new Promise((resolve, reject) => {
            const { data } = file;
            const count = Math.ceil(data.size / slicesize);

            for (let i = 1; i <= count; i++) {
                const start = (i - 1) * slicesize;
                const end = Math.min(data.size, i * slicesize);
                const fileBlob = data.slice(start, end);
                this.loadFile(fileBlob, file, i, count, resolve);
            }
        });
    }
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
                Logger('ReadFile', e);
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
            Logger('ReadFile', e);
        },
    );
}

type NotifyType = 'Processing' | 'ReadFile' | 'CheckError' | 'StartUpload' | 'UploadSucceed' | 'UploadFailed' |
'success' | 'info' | 'error' | 'warning' ;
function notify(type: NotifyType, param?: any): void {
    switch (type) {
        case 'ReadFile':
            notification.info({ className: 'drag-file-hit', message: 'Read File', duration: 5 });
            break;
        case 'CheckError':
            notification.error({ className: 'drag-file-hit', message: 'Error', description: param, placement: 'top' });
            break;
        case 'Processing':
            notification.error({
                className: 'drag-file-hit',
                message: 'Warn',
                description: 'Files are processing, Please wait for a while',
                duration: 5,
                placement: 'top',
            });
            break;
        case 'UploadSucceed':
            notification.success({
                className: 'drag-file-hit',
                message: 'Succeed:',
                description: (<div>
                    {
                        param.map((fileInfo: FileDataType) =>
                            (<div key={fileInfo.attr.index}>{`【${((fileInfo.succeed as boolean) ? 'Succeed' : 'Failed')}】${fileInfo.attr.path}`}</div>))
                    }
                </div>),
                duration: 5,
            });
            break;
        case 'UploadFailed':
            notification.error({
                className: 'drag-file-hit',
                message: 'Error:',
                duration: null,
                description: (<div>
                    {
                        param.map((fileInfo: FileDataType) =>
                            (<div key={fileInfo.attr.index}>{`【${((fileInfo.succeed as boolean) ? 'Succeed' : 'Failed')}】${fileInfo.attr.path}`}</div>))
                    }
                </div>),
            });
            break;
        default:
            notification[type](param);
            break;
    }
};

export const DragFileImportInit = ({ id, onSuccess, onDrop }: DragFileParams): DragFileImport => {
    return new DragFileImport({ id, onSuccess, onDrop });
};
