/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
import React, { useEffect } from 'react';
import { FolderFilled } from '@ant-design/icons';
import { notification, Progress } from 'antd';

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

const SLICE_SIZE = 1024 * 1024 * 10;
let uploadId = 1;

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

            async function handle(): Promise<void> {
                const res = await handleFile(e.dataTransfer.items);
                if (handleSucceed as boolean) {
                    handleSucceed(res);
                }
            }
            handle();
        }, false);
    }
};

async function handleFile(items: DataTransferItemList): Promise<void> {
    const allFiles = await getFilelist(items);
    const checkRes = checkUsable(allFiles);
    if (!checkRes.usable) {
        alert(checkRes.error);
        return;
    }
    notification.success({
        message: 'Start:',
        description: (<div style={{ width: '800px' }}>
            {
                allFiles.map((fileInfo: UploadFileDataType) =>
                    (<div key={fileInfo.info.index}>{'【File】' + fileInfo.info.path}</div>))
            }
            {`【Total File Size】${formateFileSize(checkRes.totalSize)}` }
        </div>),
        duration: null,
    });
    const result = await loopUpload(allFiles);
    notification.success({
        message: 'Succeed:',
        description: (<div style={{ width: '800px' }}>
            {
                result.map((fileInfo: UploadFileDataType) =>
                    (<div key={fileInfo.info.index}>{`【${((fileInfo.succeed as boolean) ? 'Succeed' : 'Failed')}】` + fileInfo.info.path }</div>))
            }
        </div>),
        duration: null,
    });

    return result;
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
} {
    const singleFiles = list.filter((fileInfo: UploadFileDataType) => !fileInfo.info.isInFolder);
    if (singleFiles.length > 1) {
        return {
            usable: false,
            error: 'Only Allow one File Or Folder',
        };
    }
    if (singleFiles.length === 1) {
        if ([ 'trace_view.json', 'msprof.json' ].includes(singleFiles[0].info.name)) {
            return {
                usable: true,
                totalSize: singleFiles[0].file.size,

            };
        } else {
            return {
                usable: false,
                error: 'Single File Only Allow 【trace_view.json】 or 【msprof.json】',
            };
        }
    }
    let totalSize = 0;
    list.forEach((fileInfo: UploadFileDataType) => {
        totalSize += fileInfo.file.size;
    });
    return {
        usable: true,
        totalSize,
    };
}

function formateFileSize(size = 0): string {
    const k = 1024;
    const m = 1024 * 1024;
    const g = 1024 * 1024 * 1024;
    if (size >= g) {
        return (size / g).toFixed(0) + 'GB';
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
                window.request('towingImport/action',
                    { isBinary: true, buffer, params: { ...fileInfo.info, slice: { isSliced: count > 1, index: i, count } } }).then(res => {
                    if (i >= count) {
                        resolve({ succeed: true, info: fileInfo.info, res });
                    } else {
                        resolve(readFile(fileInfo, i + 1));
                    }
                }).catch(error => {
                    // eslint-disable-next-line prefer-promise-reject-errors
                    reject({ error, succeed: false });
                });
            }
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
