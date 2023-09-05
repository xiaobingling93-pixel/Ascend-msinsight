/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { ReactNode, useEffect } from 'react';
import { FolderFilled } from '@ant-design/icons';
import { notification, Progress } from 'antd';
import './DragFile.css';

interface UploadFileDataType{
    file: any;// 二进制流
    info: {
        name: string;// 文件名
        path: string;// 文件路径
        index?: number;
        isInFolder?: boolean;
        isLast?: boolean;
        count?: number;
        [prop: string]: any;
    };
    [prop: string]: any;
}

const SLICE_SIZE = 1024 * 1024 * 20;
let uploadId = 1;

let uploading = false;
function setUploading(val: boolean): void {
    uploading = val;
}
function isUploading(): boolean {
    return uploading;
}

export const DragFileInit = (id: string, handleSucceed?: any): void => {
    const dropZone = document.getElementById(id);
    if (dropZone) {
        dropZone.addEventListener('dragenter', function (e: any) {
            e.preventDefault();
            e.stopPropagation();
        }, false);

        dropZone.addEventListener('dragover', function (e: any) {
            e.preventDefault();
            e.stopPropagation();
            e.target.classList.add('fileshover');
        }, false);

        dropZone.addEventListener('dragleave', function (e: any) {
            e.preventDefault();
            e.stopPropagation();
            e.target.classList.remove('fileshover');
        }, false);
        // 处理拖拽文件
        dropZone.addEventListener('drop', function (e: any) {
            e.preventDefault();
            e.stopPropagation();
            e.target.classList.remove('fileshover');

            handle();
            async function handle(): Promise<void> {
                if (isUploading()) {
                    notification.error({
                        className: 'drag-file-hit',
                        message: 'Warn',
                        description: 'Files are uploading, Please wait for a while',
                        duration: 5,
                        placement: 'top',
                    });
                    return;
                }
                setUploading(true);
                const result = await handleFile(e.dataTransfer.items);
                setUploading(false);
                if (handleSucceed as boolean && result as boolean) {
                    handleSucceed(result);
                }
            }
        }, false);
    }
};

async function handleFile(items: DataTransferItemList): Promise<any> {
    notification.info({
        className: 'drag-file-hit',
        message: 'Read File',
        duration: 5,
    });
    const allFiles = await getFilelist(items);
    const checkRes = checkUsable(allFiles);
    if (!checkRes.usable) {
        alertError(checkRes.error);
        return;
    }
    const { usableList = [] } = checkRes;
    notification.success({
        className: 'drag-file-hit',
        message: 'Start:',
        description: (<div>
            <div>{`【Supported File Count】${usableList.length}` }</div>
            <div>{`【Supported Total File Size】${formateFileSize(checkRes.totalSize)}` }</div>
            {
                usableList.map((fileInfo: UploadFileDataType) =>
                    (<div key={fileInfo.info.index}>{'【File】' + fileInfo.info.path}</div>))
            }
        </div>),
        duration: null,
    });

    const result: UploadFileDataType[] = await loopUpload(usableList);
    const isSucceed = result.filter((item: UploadFileDataType) => item.succeed).length === result.length;
    notification[isSucceed ? 'success' : 'error']({
        className: 'drag-file-hit',
        message: isSucceed ? 'Succeed:' : 'Error:',
        description: (<div>
            {
                result.map((fileInfo: UploadFileDataType) =>
                    (<div key={fileInfo.info.index}>{`【${((fileInfo.succeed as boolean) ? 'Succeed' : 'Failed')}】` + fileInfo.info.path }</div>))
            }
        </div>),
        duration: null,
    });
    return result[result.length - 1]?.res;
}

function alertError(description: string | ReactNode): void {
    notification.error({
        className: 'drag-file-hit',
        message: 'Error',
        description,
        placement: 'top',
    });
}

async function loopUpload(list: UploadFileDataType[], i = 0): Promise<any> {
    const reslist = [];
    for (let i = 0; i < list.length; i++) {
        const res = await readFile(list[i]);
        reslist.push(res);
    }
    return reslist;
}

async function getFilelist(items: any): Promise<any> {
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
            files.forEach((fileInfo: UploadFileDataType, index: number) => {
                fileInfo.info.index = index + 1;
                fileInfo.info.count = files.length;
                fileInfo.info.isLast = (index + 1) === files.length;
                fileInfo.info.isInFolder = fileInfo.info.path !== fileInfo.info.name;
                fileInfo.info.uploadId = uploadId;
            });
            uploadId++;
            resolve(files);
        });
    });
}

function checkUsable (list: UploadFileDataType[]): {
    usable: boolean;
    error?: string;
    totalSize?: number;
    usableList?: UploadFileDataType[];
} {
    const singleFiles = list.filter((fileInfo: UploadFileDataType) => !fileInfo.info.isInFolder);
    if (singleFiles.length > 1) {
        return {
            usable: false,
            error: 'Only Allow one File Or Folder',
        };
    }
    if (singleFiles.length === 1) {
        if (checkFileName(singleFiles[0].info.name, true)) {
            return {
                usable: true,
                totalSize: singleFiles[0].file.size,
                usableList: singleFiles,
            };
        } else {
            return {
                usable: false,
                error: 'Single File Only Allow 【trace_view.json】 or 【msprof.json】',
            };
        }
    }
    // 检查文件夹
    let totalSize = 0;
    const usableList: UploadFileDataType[] = [];
    list.forEach((fileInfo: UploadFileDataType) => {
        if (checkFileName(fileInfo.info.name)) {
            totalSize += fileInfo.file.size;
            usableList.push(fileInfo);
        }
    });
    usableList.forEach((fileInfo: UploadFileDataType, index: number) => {
        fileInfo.info.index = index + 1;
        fileInfo.info.count = usableList.length;
        fileInfo.info.isLast = (index + 1) === usableList.length;
    });

    return {
        usable: true,
        totalSize,
        usableList,
    };
}

function checkFileName(name: string, isSingle = false): boolean {
    const profReg = /^msprof\w{0,100}\.json$/;
    if (isSingle) {
        const ALL_ALLOW_FILE = [
            'trace_view.json',
            'msprof.json',
        ];
        return ALL_ALLOW_FILE.includes(name) || profReg.test(name);
    }
    const ALL_ALLOW_FILE = [
        'cluster_communication.json',
        'cluster_communication_matrix.json',
        'cluster_step_trace_time.csv',
        'communication_group.json',
        'operator_memory.csv',
        'operator_record.csv',
        'msprof.json',
        'trace_view.json',
    ];
    return ALL_ALLOW_FILE.includes(name) || profReg.test(name);
}

function formateFileSize(size = 0): string {
    const k = 1024;
    const m = 1024 * 1024;
    const g = 1024 * 1024 * 1024;
    if (size >= g) {
        return `${Number((size / g).toFixed(1))}GB`;
    }
    if (size >= m) {
        return (size / m).toFixed(0) + 'MB';
    }
    if (size >= k) {
        return (size / k).toFixed(0) + 'KB';
    }
    return String(size) + 'Byte';
}

async function getFileFromEntryRecusively(entry: any): Promise<any> {
    return new Promise((resolve, reject) => {
        if (entry.isFile === true) {
            entry.file((file: any) => {
                const fileInfo = { file, info: { path: entry.fullPath.slice(1), name: file.name } };
                resolve(fileInfo);
            }, (e: any) => {
                console.log(e);
            });
        } else {
            const reader = entry.createReader();
            reader.readEntries(
                (entries: any) => {
                    entries.forEach((entry: any) => getFileFromEntryRecusively(entry));
                    Promise.all(entries.map((entry: any) => getFileFromEntryRecusively(entry))).then(values => {
                        resolve(values.flat(1));
                    });
                },
                (e: any) => {
                    console.log(e);
                },
            );
        }
    });
}

function readFile(fileInfo: any, i = 1): Promise<any> {
    return new Promise((resolve, reject) => {
        const { file } = fileInfo;
        const count = Math.ceil(file.size / SLICE_SIZE);
        const start = (i - 1) * SLICE_SIZE;
        const end = Math.min(file.size, i * SLICE_SIZE);
        const fileBlob = fileInfo.file.slice(start, end);

        const reader = new FileReader();
        reader.readAsArrayBuffer(fileBlob);
        reader.onload = event => {
            const buffer: any = event.target?.result;
            if (buffer !== null && buffer !== undefined) {
                try {
                    window.request('towingImport/action',
                        {
                            isBinary: true,
                            buffer,
                            params: { ...fileInfo.info, slice: { isSliced: count > 1, index: i, count } },
                        }).then(res => {
                        if (i >= count) {
                            resolve({ succeed: true, info: fileInfo.info, res });
                        } else {
                            resolve(readFile(fileInfo, i + 1));
                        }
                    }).catch(error => {
                        resolve({ succeed: false, info: fileInfo.info, error });
                    });
                } catch (error) {
                    resolve({ succeed: false, info: fileInfo.info, error });
                }
            }
        };
        reader.onerror = event => {
            resolve({ succeed: false, info: fileInfo.info });
        };
    });
}

const DragFile: React.FC = () => {
    const id = 'drag';
    useEffect(() => {
        DragFileInit(id);
    }, []);

    const openSelect = async(): Promise<void> => {};
    return (
        <div className={'drag-zone'} onClick={() => openSelect()} id={id}>
            <FolderFilled className={'drag-zone-icon'}/>
            <div>点击或拖拽文件到此</div>
        </div>
    );
};

export const FileUploadList: React.FC<{fileList: UploadFileDataType[]}> = ({ fileList }) => {
    return (
        <div >
            {
                fileList.map((file: UploadFileDataType, index: number) => (
                    <div key={index}>
                        <span>{file.info.name}</span><Progress percent={file.info.progress} status={file.info.status}/>
                    </div>
                ))
            }
        </div>
    );
};

export default DragFile;
